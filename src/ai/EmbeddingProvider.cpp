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

EmbeddingProvider::EmbeddingProvider(const Config& cfg, QObject* parent)
    : QObject(parent), cfg_(cfg) {}

QList<QVector<float>> EmbeddingProvider::embedBatch(const QStringList& texts) {
    if (cfg_.provider == QLatin1String("openai")) {
        return embedOpenAICompatible(texts, QStringLiteral("https://api.openai.com/v1"));
    } else if (cfg_.provider == QLatin1String("generativa")) {
        return embedOpenAICompatible(texts, cfg_.baseUrl.isEmpty() ? QStringLiteral("https://generativa.rapport.tec.br") : cfg_.baseUrl);
    } else if (cfg_.provider == QLatin1String("ollama")) {
        return embedOllama(texts);
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
    QNetworkReply* rep = nam.post(req, body);
    QObject::connect(rep, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (rep->error() != QNetworkReply::NoError) {
        const QString err = rep->errorString(); rep->deleteLater();
        throw std::runtime_error(("HTTP error: " + err).toStdString());
    }
    const QByteArray resp = rep->readAll(); rep->deleteLater();
    const QJsonDocument doc = QJsonDocument::fromJson(resp);
    const QJsonArray data = doc.object().value("data").toArray();

    QList<QVector<float>> out; out.reserve(data.size());
    for (const auto& v : data) {
        const QJsonObject o = v.toObject();
        const QJsonArray emb = o.value("embedding").toArray();
        out.append(toVectors(QJsonArray{emb}).first());
    }
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
        QNetworkReply* rep = nam.post(req, body);
        QObject::connect(rep, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();
        if (rep->error() != QNetworkReply::NoError) {
            const QString err = rep->errorString(); rep->deleteLater();
            throw std::runtime_error(("HTTP error: " + err).toStdString());
        }
        const QByteArray resp = rep->readAll(); rep->deleteLater();
        const QJsonDocument doc = QJsonDocument::fromJson(resp);
        const QJsonArray emb = doc.object().value("embedding").toArray();
        out.append(toVectors(QJsonArray{emb}).first());
    }
    return out;
}
