#include "ui/BookProviders.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

BookProviders::BookProviders(QObject* parent) : QObject(parent) {}

static QString firstNonEmpty(const QStringList& list) {
    for (const auto& s : list) { if (!s.trimmed().isEmpty()) return s.trimmed(); }
    return QString();
}

OpfData BookProviders::parseGoogleVolume(const QJsonObject& volume) {
    OpfData out;
    const QJsonObject info = volume.value("volumeInfo").toObject();
    if (info.isEmpty()) return out;
    out.title = info.value("title").toString().trimmed();
    // Authors
    QString authors;
    const QJsonArray arrAuthors = info.value("authors").toArray();
    if (!arrAuthors.isEmpty()) {
        QStringList a;
        for (const auto& v : arrAuthors) a << v.toString();
        authors = a.join(" & ");
    }
    out.author = authors.trimmed();
    out.publisher = info.value("publisher").toString().trimmed();
    out.language = info.value("language").toString().trimmed();
    out.description = info.value("description").toString().trimmed();
    // Keywords from categories
    const QJsonArray cats = info.value("categories").toArray();
    if (!cats.isEmpty()) {
        QStringList ks; for (const auto& v : cats) ks << v.toString();
        out.keywords = ks.join(", ");
    }
    // ISBN
    const QJsonArray ids = info.value("industryIdentifiers").toArray();
    QString isbn10, isbn13;
    for (const auto& v : ids) {
        const QJsonObject o = v.toObject();
        const QString type = o.value("type").toString();
        const QString id = o.value("identifier").toString();
        if (type == QLatin1String("ISBN_13")) isbn13 = id; else if (type == QLatin1String("ISBN_10")) isbn10 = id;
    }
    out.isbn = firstNonEmpty({isbn13, isbn10});
    return out;
}

void BookProviders::fetchFromGoogleBooks(QNetworkAccessManager* nm,
                                         const OpfData& hint,
                                         std::function<void(const OpfData& result, bool ok, const QString& message)> done) {
    if (!nm) { if (done) done(OpfData{}, false, QStringLiteral("Network manager ausente")); return; }

    // Build query: prefer ISBN
    QUrl url("https://www.googleapis.com/books/v1/volumes");
    QUrlQuery q;
    if (!hint.isbn.trimmed().isEmpty()) {
        q.addQueryItem("q", QStringLiteral("isbn:%1").arg(hint.isbn));
    } else {
        QStringList parts;
        if (!hint.title.trimmed().isEmpty()) parts << QStringLiteral("intitle:%1").arg(hint.title);
        if (!hint.author.trimmed().isEmpty()) parts << QStringLiteral("inauthor:%1").arg(hint.author);
        if (parts.isEmpty()) { if (done) done(OpfData{}, false, QStringLiteral("Sem dados suficientes para consulta")); return; }
        q.addQueryItem("q", parts.join("+"));
    }
    q.addQueryItem("maxResults", "5");
    url.setQuery(q);

    QNetworkRequest req(url);
    auto* reply = nm->get(req);
    QObject::connect(reply, &QNetworkReply::finished, reply, [reply, done]() {
        const auto guard = std::unique_ptr<QNetworkReply, void(*)(QNetworkReply*)>(reply, [](QNetworkReply* r){ r->deleteLater(); });
        if (reply->error() != QNetworkReply::NoError) {
            if (done) done(OpfData{}, false, reply->errorString());
            return;
        }
        const QByteArray body = reply->readAll();
        QJsonParseError je; const QJsonDocument jd = QJsonDocument::fromJson(body, &je);
        if (je.error != QJsonParseError::NoError || !jd.isObject()) {
            if (done) done(OpfData{}, false, QStringLiteral("Resposta inválida do Google Books"));
            return;
        }
        const QJsonObject root = jd.object();
        const QJsonArray items = root.value("items").toArray();
        if (items.isEmpty()) { if (done) done(OpfData{}, false, QStringLiteral("Nenhum resultado encontrado")); return; }
        // Pick the first item for now (could be improved ranking)
        const QJsonObject vol = items.at(0).toObject();
        const OpfData parsed = BookProviders::parseGoogleVolume(vol);
        if (done) done(parsed, true, QString());
    });
}

void BookProviders::fetchFromAmazonBooks(QNetworkAccessManager* nm,
                                         const OpfData& hint,
                                         std::function<void(const OpfData& result, bool ok, const QString& message)> done) {
    Q_UNUSED(nm);
    // For now, we don't have Amazon Product Advertising API credentials configured.
    // We return a graceful message and no data, so the merge dialog still works.
    if (done) done(OpfData{}, false, QStringLiteral("Amazon Books não configurado (forneça chaves/API para habilitar)."));
}
