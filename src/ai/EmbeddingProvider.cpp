#include "ai/EmbeddingProvider.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QUrlQuery>
#include <QThread>
#include <QDebug>
#include <numeric>
#include <QRegularExpression>

static QList<QVector<float>> toVectors(const QJsonArray& arr) {
    QList<QVector<float>> out;
    out.reserve(arr.size());
    for (const auto& v : arr) {
        const QJsonArray a = v.toArray();
        QVector<float> vec; vec.reserve(a.size());
        for (const auto& e : a) vec.append(float(e.toDouble()));
        out.append(vec);
    }
    return out;
}

QList<QVector<float>> EmbeddingProvider::embedRetrievalEf(const QStringList& texts, const QString& baseUrl) {
    // Endpoint style: GET {baseUrl}/api/v1/retrieval/ef/{url-encoded-text}
    QNetworkAccessManager nam;
    QEventLoop loop;

    QList<QVector<float>> out; out.reserve(texts.size());
    for (const QString& t : texts) {
        // Normalize whitespace to avoid server 404 issues with newlines/tabs in path
        QString tNorm = t;
        tNorm.replace(QRegularExpression("\\s+"), " ");
        tNorm = tNorm.trimmed();

        const QByteArray enc = QUrl::toPercentEncoding(tNorm);
        QUrl url(baseUrl);
        QString basePath = url.path();
        if (basePath.isEmpty()) basePath = "/";
        if (!basePath.endsWith('/')) basePath += '/';
        QString path;
        if (basePath.endsWith("api/v1/", Qt::CaseInsensitive) || basePath.endsWith("api/v1", Qt::CaseInsensitive)) {
            // Base already includes /api/v1
            if (basePath.endsWith("api/v1", Qt::CaseInsensitive)) basePath += '/';
            path = basePath + "retrieval/ef/" + QString::fromUtf8(enc);
        } else {
            path = basePath + "api/v1/retrieval/ef/" + QString::fromUtf8(enc);
        }
        url.setPath(path);
        QNetworkRequest req(url);
        req.setRawHeader("Accept", "application/json");
        if (!cfg_.apiKey.isEmpty()) req.setRawHeader("Authorization", QByteArray("Bearer ") + cfg_.apiKey.toUtf8());

// removido pois está poluindo o log no terminal, 
// quando terminar o desenvolvimento desta funcionalidade, será removido.        
//        qInfo() << "[EmbeddingProvider] GET" << url.toString()
//                << "provider=retrieval-ef" << "model=" << cfg_.model
//                << "chars=" << tNorm.size() << "encoded_len=" << enc.size()
//                << "base_path=" << url.path();

        QNetworkReply* rep = nam.get(req);
        QObject::connect(rep, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();

        const int status = rep->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const QByteArray resp = rep->readAll();
        const QString errStr = rep->errorString();
        const auto netErr = rep->error();
        rep->deleteLater();

        if (netErr != QNetworkReply::NoError || status >= 400) {
            const QString bodySnippet = QString::fromUtf8(resp.left(800));
            const QString full = QStringLiteral("HTTP error: status=%1 qt_error=%2 (%3) url=%4 body=%5")
                                     .arg(status)
                                     .arg(int(netErr))
                                     .arg(errStr)
                                     .arg(url.toString())
                                     .arg(bodySnippet);
            qCritical() << "[EmbeddingProvider]" << full;
            throw std::runtime_error(full.toStdString());
        }

        const QJsonDocument doc = QJsonDocument::fromJson(resp);
        const QJsonArray arr = doc.object().value("result").toArray();
        QVector<float> vec; vec.reserve(arr.size());
        for (const auto& e : arr) vec.append(float(e.toDouble()));
        out.append(vec);
    }
    qInfo() << "[EmbeddingProvider] embeddings ok (retrieval-ef)" << "vectors=" << out.size() << "dim=" << (out.isEmpty() ? 0 : out.first().size());
    return out;
}

EmbeddingProvider::EmbeddingProvider(const Config& cfg, QObject* parent)
    : QObject(parent), cfg_(cfg) {}

QList<QVector<float>> EmbeddingProvider::embedBatch(const QStringList& texts) {
    if (cfg_.provider == QLatin1String("openai")) {
        // Allow overriding the base URL (useful for Azure/OpenAI-compatible endpoints)
        const QString base = cfg_.baseUrl.isEmpty() ? QStringLiteral("https://api.openai.com/v1") : cfg_.baseUrl;
        return embedOpenAICompatible(texts, base);
    } else if (cfg_.provider == QLatin1String("generativa")) {
        const QString base = cfg_.baseUrl.isEmpty() ? QStringLiteral("https://generativa.rapport.tec.br") : cfg_.baseUrl;
        return embedRetrievalEf(texts, base);
    } else if (cfg_.provider == QLatin1String("ollama")) {
        return embedOllama(texts);
    } else if (cfg_.provider == QLatin1String("openwebui")) {
        // Use retrieval endpoint style for OpenWebUI as specified
        const QString base = cfg_.baseUrl.isEmpty() ? QStringLiteral("http://localhost:8080") : cfg_.baseUrl;
        return embedRetrievalEf(texts, base);
    }
    throw std::runtime_error("Unsupported provider");
}

QList<QVector<float>> EmbeddingProvider::embedOpenAICompatible(const QStringList& texts, const QString& urlBase) {
    QNetworkAccessManager nam;
    QEventLoop loop;

    QUrl url(urlBase + "/embeddings");
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    if (!cfg_.apiKey.isEmpty()) req.setRawHeader("Authorization", QByteArray("Bearer ") + cfg_.apiKey.toUtf8());

    // Build payload
    QJsonObject payload;
    payload.insert("model", cfg_.model);
    QJsonArray arr;
    for (const QString& t : texts) arr.append(t);
    payload.insert("input", arr);

    const QByteArray body = QJsonDocument(payload).toJson(QJsonDocument::Compact);

    // Log request details (without sensitive headers)
    qInfo() << "[EmbeddingProvider] POST" << url.toString()
            << "provider=openai-compatible"
            << "model=" << cfg_.model
            << "batch_size=" << texts.size()
            << "input_chars_total=" << std::accumulate(texts.begin(), texts.end(), 0, [](int s, const QString& t){ return s + t.size(); });

    QNetworkReply* rep = nam.post(req, body);
    QObject::connect(rep, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    // Read response and status
    const int status = rep->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QByteArray resp = rep->readAll();
    const QString errStr = rep->errorString();
    const auto netErr = rep->error();
    rep->deleteLater();

    // Treat HTTP status >= 400 as error even if Qt did not classify it as a network error
    if (netErr != QNetworkReply::NoError || status >= 400) {
        QString details;
        // Try to parse OpenAI-style error message
        const QJsonDocument maybeJson = QJsonDocument::fromJson(resp);
        if (maybeJson.isObject()) {
            const QJsonObject obj = maybeJson.object();
            const auto errObj = obj.value("error").toObject();
            if (!errObj.isEmpty()) {
                details = QStringLiteral("message=%1 type=%2 code=%3")
                              .arg(errObj.value("message").toString())
                              .arg(errObj.value("type").toString())
                              .arg(errObj.value("code").toString());
            }
        }
        if (details.isEmpty()) {
            const QString bodySnippet = QString::fromUtf8(resp.left(800));
            details = QStringLiteral("body=%1").arg(bodySnippet);
        }
        const QString full = QStringLiteral("HTTP error: status=%1 qt_error=%2 (%3) url=%4 details=%5")
                                 .arg(status)
                                 .arg(int(netErr))
                                 .arg(errStr)
                                 .arg(url.toString())
                                 .arg(details);
        qCritical() << "[EmbeddingProvider]" << full;
        throw std::runtime_error(full.toStdString());
    }

    const QJsonDocument doc = QJsonDocument::fromJson(resp);
    const QJsonArray data = doc.object().value("data").toArray();

    QList<QVector<float>> out; out.reserve(data.size());
    for (const auto& v : data) {
        const QJsonObject o = v.toObject();
        const QJsonArray emb = o.value("embedding").toArray();
        out.append(toVectors(QJsonArray{emb}).first());
    }
    qInfo() << "[EmbeddingProvider] embeddings ok" << "vectors=" << out.size() << "dim=" << (out.isEmpty() ? 0 : out.first().size());
    return out;
}

QList<QVector<float>> EmbeddingProvider::embedOllama(const QStringList& texts) {
    QNetworkAccessManager nam;
    QEventLoop loop;

    const QString base = cfg_.baseUrl.isEmpty() ? QStringLiteral("http://localhost:11434") : cfg_.baseUrl;
    QUrl url(base + "/api/embeddings");

    QList<QVector<float>> out; out.reserve(texts.size());
    for (const QString& t : texts) {
        QNetworkRequest req(url);
        req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
        QJsonObject payload; payload.insert("model", cfg_.model); payload.insert("prompt", t);
        const QByteArray body = QJsonDocument(payload).toJson(QJsonDocument::Compact);
        qInfo() << "[EmbeddingProvider] POST" << url.toString()
                << "provider=ollama" << "model=" << cfg_.model << "chars=" << t.size();
        QNetworkReply* rep = nam.post(req, body);
        QObject::connect(rep, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();
        const int status = rep->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const QByteArray resp = rep->readAll();
        const QString errStr = rep->errorString();
        const auto netErr = rep->error();
        rep->deleteLater();
        if (netErr != QNetworkReply::NoError || status >= 400) {
            const QString bodySnippet = QString::fromUtf8(resp.left(800));
            const QString full = QStringLiteral("HTTP error: status=%1 qt_error=%2 (%3) url=%4 body=%5")
                                     .arg(status)
                                     .arg(int(netErr))
                                     .arg(errStr)
                                     .arg(url.toString())
                                     .arg(bodySnippet);
            qCritical() << "[EmbeddingProvider]" << full;
            throw std::runtime_error(full.toStdString());
        }
        
        const QJsonDocument doc = QJsonDocument::fromJson(resp);
        const QJsonArray emb = doc.object().value("embedding").toArray();
        out.append(toVectors(QJsonArray{emb}).first());
    }
    qInfo() << "[EmbeddingProvider] embeddings ok (ollama)" << "vectors=" << out.size() << "dim=" << (out.isEmpty() ? 0 : out.first().size());
    return out;
}
