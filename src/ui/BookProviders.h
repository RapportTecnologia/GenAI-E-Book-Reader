#pragma once

#include <QObject>
#include <QJsonObject>
#include <functional>

class QNetworkAccessManager;

#include "ui/OpfStore.h"

/**
 * Helper to enrich OPF metadata using external book info providers (e.g., Google Books).
 */
class BookProviders : public QObject {
    Q_OBJECT
public:
    explicit BookProviders(QObject* parent = nullptr);

    // Query Google Books using ISBN when available; otherwise use title and author.
    // Calls done(result, ok, message) on completion. 'result' includes only fields found.
    void fetchFromGoogleBooks(QNetworkAccessManager* nm,
                              const OpfData& hint,
                              std::function<void(const OpfData& result, bool ok, const QString& message)> done);

    // Query Amazon Books (Product Advertising API or public pages). This is a placeholder implementation
    // that returns an informative message until API keys/config are provided.
    // Calls done(result, ok, message).
    void fetchFromAmazonBooks(QNetworkAccessManager* nm,
                              const OpfData& hint,
                              std::function<void(const OpfData& result, bool ok, const QString& message)> done);

private:
    static OpfData parseGoogleVolume(const QJsonObject& volume);
};
