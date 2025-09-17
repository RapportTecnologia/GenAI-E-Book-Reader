#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>

// Simple provider interface for generating embeddings from text batches.
class EmbeddingProvider : public QObject {
    Q_OBJECT
public:
    struct Config {
        QString provider;   // "openai", "generativa", "ollama", "openwebui"
        QString model;
        QString baseUrl;    // used for openai-compatible endpoints (generativa)
        QString apiKey;     // for openai/generativa
    };

    explicit EmbeddingProvider(const Config& cfg, QObject* parent = nullptr);

    // Returns NxD embeddings. Throws on error (via exceptions) with a descriptive message.
    QList<QVector<float>> embedBatch(const QStringList& texts);

private:
    QList<QVector<float>> embedOpenAICompatible(const QStringList& texts, const QString& urlBase);
    QList<QVector<float>> embedOllama(const QStringList& texts);
    QList<QVector<float>> embedRetrievalEf(const QStringList& texts, const QString& baseUrl);

    Config cfg_;
};
