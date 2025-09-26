#include "ai/LlmClient.h"

#include <QSettings>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QCoreApplication>
#include <QDebug>

LlmClient::LlmClient(QObject* parent) : QObject(parent) {
    nam_ = new QNetworkAccessManager(this);
    reloadSettings();
}

void LlmClient::postJsonForTools(const QUrl& url, const QJsonObject& body,
                                 std::function<void(QString, QJsonArray, QString)> onFinished) {
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    if (!apiKey_.isEmpty()) {
        req.setRawHeader("Authorization", QByteArray("Bearer ") + apiKey_.toUtf8());
    }
    if (provider_ == QLatin1String("openrouter")) {
        const QByteArray referer = QByteArray("https://rapport.tec.br/genai-e-book-reader");
        req.setRawHeader("HTTP-Referer", referer);
        const QByteArray title = QCoreApplication::applicationName().isEmpty()
                                    ? QByteArray("GenAI E-Book Reader")
                                    : QCoreApplication::applicationName().toUtf8();
        req.setRawHeader("X-Title", title);
    }
    const QByteArray data = QJsonDocument(body).toJson(QJsonDocument::Compact);
    qInfo().noquote() << "[LlmClient][HTTP][POST][tools]" << url.toString() << "provider=" << provider_ << " payload_size=" << data.size();
    auto* reply = nam_->post(req, data);
    QObject::connect(reply, &QNetworkReply::finished, this, [this, reply, onFinished]() {
        const auto guard = std::unique_ptr<QNetworkReply, void(*)(QNetworkReply*)>(reply, [](QNetworkReply* r){ r->deleteLater(); });
        if (reply->error() != QNetworkReply::NoError) {
            const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            const QByteArray raw = reply->readAll();
            QString body = QString::fromUtf8(raw).trimmed();
            if (body.size() > 1200) body = body.left(1200) + "...";
            QString err = QString("HTTP %1 — %2\n%3").arg(status).arg(reply->errorString(), body);
            qWarning().noquote() << "[LlmClient][HTTP][ERROR][tools] status=" << status << " err=" << reply->errorString();
            onFinished(QString(), QJsonArray(), err);
            return;
        }
        const QByteArray raw = reply->readAll();
        qInfo().noquote() << "[LlmClient][HTTP][OK][tools] bytes=" << raw.size();
        const QJsonDocument doc = QJsonDocument::fromJson(raw);
        if (!doc.isObject()) { onFinished(QString::fromUtf8(raw), QJsonArray(), QString()); return; }
        const QJsonObject obj = doc.object();
        QString content;
        QJsonArray toolCalls;
        if (provider_ == QLatin1String("ollama")) {
            // No tool support here; mirror regular content extraction
            if (obj.contains("message")) {
                const auto m = obj.value("message").toObject();
                content = m.value("content").toString();
            }
            if (content.isEmpty()) content = obj.value("response").toString();
        } else {
            if (obj.contains("choices")) {
                const auto arr = obj.value("choices").toArray();
                if (!arr.isEmpty()) {
                    const auto choice = arr.first().toObject();
                    const auto msg = choice.value("message").toObject();
                    content = msg.value("content").toString();
                    // OpenAI-style: message.tool_calls is an array
                    const auto tc = msg.value("tool_calls");
                    if (tc.isArray()) toolCalls = tc.toArray();
                }
            }
        }
        onFinished(content, toolCalls, QString());
    });
}

void LlmClient::chatWithMessages(const QList<QPair<QString, QString>>& messagesIn,
                                 std::function<void(QString, QString)> onFinished) {
    // Choose endpoint per provider
    const QUrl url(
        provider_ == QLatin1String("ollama")
            ? QUrl(baseUrl_ + "/chat")
            : (provider_ == QLatin1String("perplexity")
                   ? QUrl(baseUrl_ + "/chat/completions")
                   : QUrl(baseUrl_ + "/v1/chat/completions"))
    );
    QJsonObject body; body["model"] = model_;
    QJsonArray messages;
    // Always prepend a MathJax/LaTeX directive to avoid ambiguity in rendering formulas
    const QString mathDirective = QStringLiteral(
        "Quando incluir fórmulas ou equações matemáticas nas respostas, formate-as usando LaTeX adequado ao MathJax: "
        "use $...$ para inline e $$...$$ para exibição. Você também pode usar \\(...\\) e \\[...\\]. "
        "Não use imagens para fórmulas; sempre use notação LaTeX renderizável."
    );
    {
        QJsonObject sysMsg; sysMsg["role"] = "system"; sysMsg["content"] = mathDirective; messages.append(sysMsg);
    }
    bool hasSystem = false;
    for (const auto& rc : messagesIn) {
        const QString role = rc.first.trimmed().toLower();
        const QString content = rc.second;
        QJsonObject m; m["role"] = role; m["content"] = content; messages.append(m);
        if (role == QLatin1String("system")) hasSystem = true;
    }
    if (!hasSystem) {
        QSettings s;
        const QString sys = s.value("ai/prompts/chat").toString();
        if (!sys.trimmed().isEmpty()) {
            QJsonObject sysMsg; sysMsg["role"] = "system"; sysMsg["content"] = sys;
            messages.prepend(sysMsg);
        }
        // Optional: personalize with user's nickname
        const QString nick = s.value("reader/nickname").toString().trimmed();
        if (!nick.isEmpty()) {
            QJsonObject sysNick; sysNick["role"] = "system";
            sysNick["content"] = QStringLiteral("Quando apropriado, trate o usuário pelo apelido '%1'.").arg(nick);
            messages.prepend(sysNick);
        }
    }
    if (provider_ == QLatin1String("ollama")) {
        // Ollama chat format: {model, messages:[{role, content}], stream:false}
        body["messages"] = messages;
        body["stream"] = false;
        postJson(url, body, onFinished);
        return;
    }
    body["messages"] = messages;
    postJson(url, body, onFinished);
}

void LlmClient::chatWithMessagesTools(const QList<QPair<QString, QString>>& messagesIn,
                                      const QJsonArray& tools,
                                      std::function<void(QString, QJsonArray, QString)> onFinished) {
    // Build OpenAI-compatible request with tools. Providers that ignore will just return normal content.
    const QUrl url(provider_ == QLatin1String("ollama")
                       ? QUrl(baseUrl_ + "/chat")
                       : QUrl(baseUrl_ + "/v1/chat/completions"));
    QJsonObject body; body["model"] = model_;
    QJsonArray messages;
    // Always prepend MathJax directive
    const QString mathDirective = QStringLiteral(
        "Quando incluir fórmulas ou equações matemáticas nas respostas, formate-as usando LaTeX adequado ao MathJax: "
        "use $...$ para inline e $$...$$ para exibição. Você também pode usar \\(..\\) e \\[...\\]. "
        "Não use imagens para fórmulas; sempre use notação LaTeX renderizável.");
    {
        QJsonObject sysMsg; sysMsg["role"] = "system"; sysMsg["content"] = mathDirective; messages.append(sysMsg);
    }
    bool hasSystem = false;
    for (const auto& rc : messagesIn) {
        const QString role = rc.first.trimmed().toLower();
        const QString content = rc.second;
        QJsonObject m; m["role"] = role; m["content"] = content; messages.append(m);
        if (role == QLatin1String("system")) hasSystem = true;
    }
    if (!hasSystem) {
        QSettings s;
        const QString sys = s.value("ai/prompts/chat").toString();
        if (!sys.trimmed().isEmpty()) { QJsonObject sysMsg; sysMsg["role"] = "system"; sysMsg["content"] = sys; messages.prepend(sysMsg); }
        const QString nick = s.value("reader/nickname").toString().trimmed();
        if (!nick.isEmpty()) { QJsonObject sysNick; sysNick["role"] = "system"; sysNick["content"] = QStringLiteral("Quando apropriado, trate o usuário pelo apelido '%1'.").arg(nick); messages.prepend(sysNick); }
    }
    body["messages"] = messages;
    if (provider_ != QLatin1String("ollama")) {
        // Only OpenAI-style endpoints support tools (body field). Ollama has a different tools API not used here.
        if (!tools.isEmpty()) body["tools"] = tools;
    }
    postJsonForTools(url, body, onFinished);
}

void LlmClient::chatWithImage(const QString& userPrompt, const QString& imageDataUrl, std::function<void(QString, QString)> onFinished) {
    if (provider_ == QLatin1String("ollama")) {
        onFinished(QString(), QStringLiteral("O provedor Ollama (local) não suporta chat com imagem neste aplicativo."));
        return;
    }
    const QUrl url(provider_ == QLatin1String("perplexity")
                       ? QUrl(baseUrl_ + "/chat/completions")
                       : QUrl(baseUrl_ + "/v1/chat/completions"));
    QJsonObject body; body["model"] = model_;
    QJsonArray messages;
    // Optional system prompt
    {
        // Always add MathJax/LaTeX directive first
        const QString mathDirective = QStringLiteral(
            "Quando incluir fórmulas ou equações matemáticas nas respostas, formate-as usando LaTeX adequado ao MathJax: "
            "use $...$ para inline e $$...$$ para exibição. Você também pode usar \\(...\\) e \\[...\\]. "
            "Não use imagens para fórmulas; sempre use notação LaTeX renderizável."
        );
        QJsonObject sysMath; sysMath["role"] = "system"; sysMath["content"] = mathDirective; messages.append(sysMath);

        QSettings s;
        const QString sys = s.value("ai/prompts/chat").toString();
        if (!sys.trimmed().isEmpty()) { QJsonObject sysMsg; sysMsg["role"] = "system"; sysMsg["content"] = sys; messages.append(sysMsg); }
        const QString nick = s.value("reader/nickname").toString().trimmed();
        if (!nick.isEmpty()) { QJsonObject sysNick; sysNick["role"] = "system"; sysNick["content"] = QStringLiteral("Quando apropriado, trate o usuário pelo apelido '%1'.").arg(nick); messages.append(sysNick); }
    }
    // User content: text + image_url
    {
        QJsonArray content;
        QJsonObject partText; partText["type"] = "text"; partText["text"] = userPrompt;
        content.append(partText);
        QJsonObject partImg; partImg["type"] = "image_url";
        QJsonObject imgObj; imgObj["url"] = imageDataUrl;
        partImg["image_url"] = imgObj;
        content.append(partImg);
        QJsonObject m; m["role"] = "user"; m["content"] = content;
        messages.append(m);
    }
    body["messages"] = messages;
    postJson(url, body, onFinished);
}

void LlmClient::reloadSettings() {
    QSettings s;
    provider_ = s.value("ai/provider", "openai").toString();
    // Migration: move legacy ai/api_key to provider-specific ai/api_keys/<provider>
    const QString perProviderKey = QStringLiteral("ai/api_keys/%1").arg(provider_);
    if (s.contains("ai/api_key") && !s.contains(perProviderKey)) {
        const QString legacy = s.value("ai/api_key").toString();
        if (!legacy.isEmpty()) {
            s.setValue(perProviderKey, legacy);
            // Optional: keep legacy for backward-compat; comment next line to retain
            s.remove("ai/api_key");
        }
    }
    // Load API key (provider-specific first, then env fallback per provider)
    if (s.contains(perProviderKey)) {
        apiKey_ = s.value(perProviderKey).toString();
    } else if (provider_ == QLatin1String("openrouter")) {
        apiKey_ = QString::fromUtf8(qgetenv("OPENROUTER_API_KEY"));
    } else if (provider_ == QLatin1String("generativa")) {
        apiKey_ = QString::fromUtf8(qgetenv("GENERATIVA_API_KEY"));
        if (apiKey_.isEmpty()) apiKey_ = QString::fromUtf8(qgetenv("OPENAI_API_KEY"));
    } else if (provider_ == QLatin1String("perplexity")) {
        apiKey_ = QString::fromUtf8(qgetenv("PERPLEXITY_API_KEY"));
    } else {
        apiKey_ = QString::fromUtf8(qgetenv("OPENAI_API_KEY"));
    }
    // Default model per provider
    if (s.contains("ai/model")) {
        model_ = s.value("ai/model").toString();
    } else {
        if (provider_ == QLatin1String("generativa")) {
            model_ = "granite3.2:8b";
        } else if (provider_ == QLatin1String("openrouter")) {
            // OpenRouter auto-routing model
            model_ = "openrouter/auto";
        } else if (provider_ == QLatin1String("ollama")) {
            model_ = "granite3.2:8b";
        } else if (provider_ == QLatin1String("openwebui")) {
            model_ = "granite3.2:8b";
        } else if (provider_ == QLatin1String("perplexity")) {
            // Default to an online-search-enabled model
            model_ = "sonar-small-online";
        } else {
            model_ = "gpt-4o-mini";
        }
    }

    const QString overrideBase = s.value("ai/base_url").toString();
    if (!overrideBase.trimmed().isEmpty()) {
        baseUrl_ = overrideBase.trimmed();
    } else if (provider_ == "generativa") {
        baseUrl_ = "https://generativa.rapport.tec.br/api";
    } else if (provider_ == "openrouter") {
        baseUrl_ = "https://openrouter.ai/api";
    } else if (provider_ == "ollama") {
        baseUrl_ = "http://localhost:11434/api";
    } else if (provider_ == "openwebui") {
        baseUrl_ = "http://localhost:3000/api";
    } else if (provider_ == "perplexity") {
        // Perplexity uses /chat/completions (no /v1 prefix)
        baseUrl_ = "https://api.perplexity.ai";
    } else {
        baseUrl_ = "https://api.openai.com";
    }

    qInfo().noquote() << "[LlmClient][reloadSettings] provider=" << provider_
                      << " model=" << model_
                      << " baseUrl=" << baseUrl_
                      << " apiKeySet=" << (!apiKey_.isEmpty());
}

void LlmClient::postJson(const QUrl& url, const QJsonObject& body, std::function<void(QString, QString)> onFinished) {
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    if (!apiKey_.isEmpty()) {
        req.setRawHeader("Authorization", QByteArray("Bearer ") + apiKey_.toUtf8());
    }
    // OpenRouter recommends/requests these headers for attribution and to reduce abuse
    if (provider_ == QLatin1String("openrouter")) {
        // If you have an app/site URL, set it here. Fallback to application name.
        const QByteArray referer = QByteArray("https://rapport.tec.br/genai-e-book-reader");
        req.setRawHeader("HTTP-Referer", referer);
        const QByteArray title = QCoreApplication::applicationName().isEmpty()
                                    ? QByteArray("GenAI E-Book Reader")
                                    : QCoreApplication::applicationName().toUtf8();
        req.setRawHeader("X-Title", title);
    }
    const QByteArray data = QJsonDocument(body).toJson(QJsonDocument::Compact);
    qInfo().noquote() << "[LlmClient][HTTP][POST]" << url.toString() << "provider=" << provider_ << " payload_size=" << data.size();
    auto* reply = nam_->post(req, data);
    QObject::connect(reply, &QNetworkReply::finished, this, [this, reply, onFinished]() {
        const auto guard = std::unique_ptr<QNetworkReply, void(*)(QNetworkReply*)>(reply, [](QNetworkReply* r){ r->deleteLater(); });
        if (reply->error() != QNetworkReply::NoError) {
            const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            const QByteArray raw = reply->readAll();
            QString body = QString::fromUtf8(raw).trimmed();
            if (body.size() > 1200) body = body.left(1200) + "...";
            QString err = QString("HTTP %1 — %2\n%3").arg(status).arg(reply->errorString(), body);
            qWarning().noquote() << "[LlmClient][HTTP][ERROR] status=" << status << " err=" << reply->errorString();
            onFinished(QString(), err);
            return;
        }
        const QByteArray raw = reply->readAll();
        qInfo().noquote() << "[LlmClient][HTTP][OK] bytes=" << raw.size();
        const QJsonDocument doc = QJsonDocument::fromJson(raw);
        if (!doc.isObject()) { onFinished(QString::fromUtf8(raw), QString()); return; }
        const QJsonObject obj = doc.object();
        // Parse per provider
        QString content;
        if (provider_ == QLatin1String("ollama")) {
            // /api/chat: { message: { role, content }, done: true }
            if (obj.contains("message")) {
                const auto m = obj.value("message").toObject();
                content = m.value("content").toString();
            }
            if (content.isEmpty()) {
                // /api/generate compatibility: { response: "..." }
                content = obj.value("response").toString();
            }
        } else {
            // OpenAI-style extraction
            if (obj.contains("choices")) {
                const auto arr = obj.value("choices").toArray();
                if (!arr.isEmpty()) {
                    const auto choice = arr.first().toObject();
                    const auto msg = choice.value("message").toObject();
                    content = msg.value("content").toString();
                }
            }
        }
        if (content.isEmpty()) content = QString::fromUtf8(raw);
        onFinished(content, QString());
    });
}

void LlmClient::chat(const QString& userMessage, std::function<void(QString, QString)> onFinished) {
    const QUrl url(provider_ == QLatin1String("ollama")
                       ? QUrl(baseUrl_ + "/chat")
                       : QUrl(baseUrl_ + "/v1/chat/completions"));
    QJsonObject body;
    body["model"] = model_;
    QJsonArray messages;
    // Optional system prompt for chat
    {
        // MathJax/LaTeX directive to ensure math formatting
        const QString mathDirective = QStringLiteral(
            "Quando incluir fórmulas ou equações matemáticas nas respostas, formate-as usando LaTeX adequado ao MathJax: "
            "use $...$ para inline e $$...$$ para exibição. Você também pode usar \\(...\\) e \\[...\\]. "
            "Não use imagens para fórmulas; sempre use notação LaTeX renderizável."
        );
        { QJsonObject sysMsg; sysMsg["role"] = "system"; sysMsg["content"] = mathDirective; messages.append(sysMsg); }

        QSettings s;
        const QString sys = s.value("ai/prompts/chat").toString();
        if (!sys.trimmed().isEmpty()) {
            QJsonObject sysMsg; sysMsg["role"] = "system"; sysMsg["content"] = sys;
            messages.append(sysMsg);
        }
    }
    {
        QJsonObject m; m["role"] = "user"; m["content"] = userMessage; messages.append(m);
    }
    body["messages"] = messages;
    postJson(url, body, onFinished);
}

void LlmClient::summarize(const QString& text, std::function<void(QString, QString)> onFinished) {
    const QUrl url(provider_ == QLatin1String("ollama")
                       ? QUrl(baseUrl_ + "/chat")
                       : QUrl(baseUrl_ + "/v1/chat/completions"));
    QJsonObject body; body["model"] = model_;
    QJsonArray messages;
    // system from settings
    {
        // MathJax directive for potential formulas in summaries
        const QString mathDirective = QStringLiteral(
            "Quando incluir fórmulas ou equações matemáticas nas respostas, formate-as usando LaTeX adequado ao MathJax: "
            "use $...$ para inline e $$...$$ para exibição. Você também pode usar \\(...\\) e \\[...\\]. "
            "Não use imagens para fórmulas; sempre use notação LaTeX renderizável."
        );
        { QJsonObject sysMsg; sysMsg["role"] = "system"; sysMsg["content"] = mathDirective; messages.append(sysMsg); }

        QSettings s;
        const QString sys = s.value("ai/prompts/summaries").toString();
        if (!sys.trimmed().isEmpty()) { QJsonObject sysMsg; sysMsg["role"] = "system"; sysMsg["content"] = sys; messages.append(sysMsg); }
        const QString nick = s.value("reader/nickname").toString().trimmed();
        if (!nick.isEmpty()) { QJsonObject sysNick; sysNick["role"] = "system"; sysNick["content"] = QStringLiteral("Quando apropriado, trate o usuário pelo apelido '%1'.").arg(nick); messages.append(sysNick); }
    }
    // user content
    {
        QJsonObject m; m["role"] = "user";
        m["content"] = QString::fromLatin1("Trecho a resumir:\n%1").arg(text);
        messages.append(m);
    }
    body["messages"] = messages;
    postJson(url, body, onFinished);
}

void LlmClient::synonyms(const QString& wordOrLocution, const QString& locale, std::function<void(QString, QString)> onFinished) {
    const QUrl url(provider_ == QLatin1String("ollama")
                       ? QUrl(baseUrl_ + "/chat")
                       : QUrl(baseUrl_ + "/v1/chat/completions"));
    QJsonObject body; body["model"] = model_;
    QJsonArray messages;
    {
        // MathJax directive in case definitions or examples include math
        const QString mathDirective = QStringLiteral(
            "Quando incluir fórmulas ou equações matemáticas nas respostas, formate-as usando LaTeX adequado ao MathJax: "
            "use $...$ para inline e $$...$$ para exibição. Você também pode usar \\(...\\) e \\[...\\]. "
            "Não use imagens para fórmulas; sempre use notação LaTeX renderizável."
        );
        { QJsonObject sysMsg; sysMsg["role"] = "system"; sysMsg["content"] = mathDirective; messages.append(sysMsg); }

        QSettings s;
        const QString sys = s.value("ai/prompts/synonyms").toString();
        if (!sys.trimmed().isEmpty()) { QJsonObject sysMsg; sysMsg["role"] = "system"; sysMsg["content"] = sys; messages.append(sysMsg); }
        const QString nick = s.value("reader/nickname").toString().trimmed();
        if (!nick.isEmpty()) { QJsonObject sysNick; sysNick["role"] = "system"; sysNick["content"] = QStringLiteral("Quando apropriado, trate o usuário pelo apelido '%1'.").arg(nick); messages.append(sysNick); }
    }
    {
        const QString loc = locale.isEmpty() ? QStringLiteral("pt-BR") : locale;
        QJsonObject m; m["role"] = "user"; m["content"] = QString::fromLatin1("Idioma: %1\nTermo: %2").arg(loc, wordOrLocution);
        messages.append(m);
    }
    body["messages"] = messages;
    postJson(url, body, onFinished);
}
