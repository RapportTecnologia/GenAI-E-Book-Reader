#pragma once

/**
 * \file LlmClient.h
 * \brief Cliente simples compatível com a API de chat do OpenAI (e GenerAtiva).
 *
 * Esta classe centraliza chamadas HTTP para provedores OpenAI-compatíveis (OpenAI, GenerAtiva,
 * OpenRouter, OpenWebUI, Ollama) e também Perplexity ("OpenAI-like" com caminhos levemente
 * diferentes),
 * lendo configurações de \c QSettings e expondo métodos utilitários para chat, sumarização,
 * sinônimos e chat multimodal (imagem).
 * \ingroup ai
 */

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
    //   ai/provider: "openai" | "generativa" | "openrouter" | "openwebui" | "ollama" | "perplexity" (default: openai)
    //   ai/base_url: base URL override (optional)
    //   ai/api_key: secret token
    //   ai/model: model name (default: gpt-4o-mini or gpt-3.5-turbo compatible)
    /** \brief Recarrega configurações do cliente a partir de QSettings. */
    void reloadSettings();

    // Send a chat completion request with a single user message
    /** \brief Envia um chat com uma única mensagem do usuário. Retorna conteúdo e erro. */
    void chat(const QString& userMessage, std::function<void(QString, QString)> onFinished); // (content, error)

    // Send a chat completion request with the full history (role, content). Roles: "system"|"user"|"assistant"
    /** \brief Envia um chat com histórico completo (pares role, content). */
    void chatWithMessages(const QList<QPair<QString, QString>>& messages,
                          std::function<void(QString, QString)> onFinished);

    // Chat with optional OpenAI-style tools (Function Calling). If the model returns tool calls,
    // they will be provided in toolCalls (as an array of {id,type,function:{name,arguments}}).
    // Providers that do not support tools will simply return content and an empty toolCalls array.
    /** \brief Envia chat com ferramentas (Function Calling); retorna conteúdo e toolCalls. */
    void chatWithMessagesTools(const QList<QPair<QString, QString>>& messages,
                               const QJsonArray& tools,
                               std::function<void(QString /*content*/, QJsonArray /*toolCalls*/, QString /*error*/)> onFinished);

    // Helper prompts
    /** \brief Gera um resumo curto de \p text. */
    void summarize(const QString& text, std::function<void(QString, QString)> onFinished);
    /** \brief Sugere sinônimos para uma palavra ou locução, considerando o locale. */
    void synonyms(const QString& wordOrLocution, const QString& locale, std::function<void(QString, QString)> onFinished);
    /** \brief Realiza chat multimodal com uma imagem embutida como data URL. */
    void chatWithImage(const QString& userPrompt, const QString& imageDataUrl, std::function<void(QString, QString)> onFinished);

private:
    QString baseUrl_;
    QString apiKey_;
    QString model_;
    QString provider_;

    QNetworkAccessManager* nam_ {nullptr};

    /** \brief POST JSON helper (sem ferramentas). */
    void postJson(const QUrl& url, const QJsonObject& body, std::function<void(QString, QString)> onFinished);
    /** \brief POST JSON helper que extrai \c toolCalls no retorno do provider. */
    void postJsonForTools(const QUrl& url, const QJsonObject& body,
                          std::function<void(QString, QJsonArray, QString)> onFinished);
};

