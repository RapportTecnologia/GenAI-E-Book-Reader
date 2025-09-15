#include "ai/LlmClient.h"

#include <QSettings>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QCoreApplication>

LlmClient::LlmClient(QObject* parent) : QObject(parent) {
    nam_ = new QNetworkAccessManager(this);
    reloadSettings();
}

void LlmClient::chatWithMessages(const QList<QPair<QString, QString>>& messagesIn,
                                 std::function<void(QString, QString)> onFinished) {
    const QUrl url(baseUrl_ + "/v1/chat/completions");
    QJsonObject body; body["model"] = model_;
    QJsonArray messages;
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
    }
    body["messages"] = messages;
    postJson(url, body, onFinished);
}

void LlmClient::chatWithImage(const QString& userPrompt, const QString& imageDataUrl, std::function<void(QString, QString)> onFinished) {
    const QUrl url(baseUrl_ + "/v1/chat/completions");
    QJsonObject body; body["model"] = model_;
    QJsonArray messages;
    // Optional system prompt
    {
        QSettings s;
        const QString sys = s.value("ai/prompts/chat").toString();
        if (!sys.trimmed().isEmpty()) {
            QJsonObject sysMsg; sysMsg["role"] = "system"; sysMsg["content"] = sys; messages.append(sysMsg);
        }
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
    apiKey_ = s.value("ai/api_key", QString::fromUtf8(qgetenv("OPENAI_API_KEY"))).toString();
    model_ = s.value("ai/model", provider_ == "generativa" ? "gpt-4o-mini" : "gpt-4o-mini").toString();

    const QString overrideBase = s.value("ai/base_url").toString();
    if (!overrideBase.trimmed().isEmpty()) {
        baseUrl_ = overrideBase.trimmed();
    } else if (provider_ == "generativa") {
        baseUrl_ = "https://generativa.rapport.tec.br";
    } else {
        baseUrl_ = "https://api.openai.com";
    }
}

void LlmClient::postJson(const QUrl& url, const QJsonObject& body, std::function<void(QString, QString)> onFinished) {
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    if (!apiKey_.isEmpty()) {
        req.setRawHeader("Authorization", QByteArray("Bearer ") + apiKey_.toUtf8());
    }
    const QByteArray data = QJsonDocument(body).toJson(QJsonDocument::Compact);
    auto* reply = nam_->post(req, data);
    QObject::connect(reply, &QNetworkReply::finished, this, [reply, onFinished]() {
        const auto guard = std::unique_ptr<QNetworkReply, void(*)(QNetworkReply*)>(reply, [](QNetworkReply* r){ r->deleteLater(); });
        if (reply->error() != QNetworkReply::NoError) {
            const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            const QByteArray raw = reply->readAll();
            QString body = QString::fromUtf8(raw).trimmed();
            if (body.size() > 1200) body = body.left(1200) + "...";
            QString err = QString("HTTP %1 — %2\n%3").arg(status).arg(reply->errorString(), body);
            onFinished(QString(), err);
            return;
        }
        const QByteArray raw = reply->readAll();
        const QJsonDocument doc = QJsonDocument::fromJson(raw);
        if (!doc.isObject()) { onFinished(QString::fromUtf8(raw), QString()); return; }
        const QJsonObject obj = doc.object();
        // OpenAI-style extraction
        QString content;
        if (obj.contains("choices")) {
            const auto arr = obj.value("choices").toArray();
            if (!arr.isEmpty()) {
                const auto choice = arr.first().toObject();
                const auto msg = choice.value("message").toObject();
                content = msg.value("content").toString();
            }
        }
        if (content.isEmpty()) content = QString::fromUtf8(raw);
        onFinished(content, QString());
    });
}

void LlmClient::chat(const QString& userMessage, std::function<void(QString, QString)> onFinished) {
    const QUrl url(baseUrl_ + "/v1/chat/completions");
    QJsonObject body;
    body["model"] = model_;
    QJsonArray messages;
    // Optional system prompt for chat
    {
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
    const QUrl url(baseUrl_ + "/v1/chat/completions");
    QJsonObject body; body["model"] = model_;
    QJsonArray messages;
    // system from settings
    {
        QSettings s;
        const QString sys = s.value("ai/prompts/summaries").toString();
        if (!sys.trimmed().isEmpty()) { QJsonObject sysMsg; sysMsg["role"] = "system"; sysMsg["content"] = sys; messages.append(sysMsg); }
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
    const QUrl url(baseUrl_ + "/v1/chat/completions");
    QJsonObject body; body["model"] = model_;
    QJsonArray messages;
    {
        QSettings s;
        const QString sys = s.value("ai/prompts/synonyms").toString();
        if (!sys.trimmed().isEmpty()) { QJsonObject sysMsg; sysMsg["role"] = "system"; sysMsg["content"] = sys; messages.append(sysMsg); }
    }
    {
        const QString loc = locale.isEmpty() ? QStringLiteral("pt-BR") : locale;
        QJsonObject m; m["role"] = "user"; m["content"] = QString::fromLatin1("Idioma: %1\nTermo: %2").arg(loc, wordOrLocution);
        messages.append(m);
    }
    body["messages"] = messages;
    postJson(url, body, onFinished);
}
