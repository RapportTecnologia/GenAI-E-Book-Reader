#pragma once

#include <QObject>
#include <QString>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QJsonObject>
#include <QJsonArray>
#include <functional>

// Simple OpenAI-compatible client. Supports custom base URL for GenerAtiva
// Default provider: OpenAI (https://api.openai.com)
// GenerAtiva provider: https://generativa.rapport.tec.br
class LlmClient : public QObject {
    Q_OBJECT
public:
    explicit LlmClient(QObject* parent = nullptr);

    // Configuration is read first from QSettings keys:
    //   ai/provider: "openai" | "generativa" (default: openai)
    //   ai/base_url: base URL override (optional)
    //   ai/api_key: secret token
    //   ai/model: model name (default: gpt-4o-mini or gpt-3.5-turbo compatible)
    void reloadSettings();

    // Send a chat completion request with a single user message
    void chat(const QString& userMessage, std::function<void(QString, QString)> onFinished); // (content, error)

    // Send a chat completion request with the full history (role, content). Roles: "system"|"user"|"assistant"
    void chatWithMessages(const QList<QPair<QString, QString>>& messages,
                          std::function<void(QString, QString)> onFinished);

    // Chat with optional OpenAI-style tools (Function Calling). If the model returns tool calls,
    // they will be provided in toolCalls (as an array of {id,type,function:{name,arguments}}).
    // Providers that do not support tools will simply return content and an empty toolCalls array.
    void chatWithMessagesTools(const QList<QPair<QString, QString>>& messages,
                               const QJsonArray& tools,
                               std::function<void(QString /*content*/, QJsonArray /*toolCalls*/, QString /*error*/)> onFinished);

    // Helper prompts
    void summarize(const QString& text, std::function<void(QString, QString)> onFinished);
    void synonyms(const QString& wordOrLocution, const QString& locale, std::function<void(QString, QString)> onFinished);
    void chatWithImage(const QString& userPrompt, const QString& imageDataUrl, std::function<void(QString, QString)> onFinished);

private:
    QString baseUrl_;
    QString apiKey_;
    QString model_;
    QString provider_;

    QNetworkAccessManager* nam_ {nullptr};

    void postJson(const QUrl& url, const QJsonObject& body, std::function<void(QString, QString)> onFinished);
    void postJsonForTools(const QUrl& url, const QJsonObject& body,
                          std::function<void(QString, QJsonArray, QString)> onFinished);
};
