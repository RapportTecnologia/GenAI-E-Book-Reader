 #pragma once

#include <QObject>
#include <QtGlobal>

#ifdef HAVE_QT_WEBENGINE

#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QString>
#include <QDebug>

// Lightweight helper page that logs JS console messages and allows us to
// share a configured QWebEngineProfile across views.
class WebPage : public QWebEnginePage {
public:
    explicit WebPage(QWebEngineProfile* profile, QObject* parent = nullptr)
        : QWebEnginePage(profile, parent) {}
    virtual ~WebPage() override = default;

protected:
#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
    void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level,
                                  const QString& message,
                                  int lineNumber,
                                  const QString& sourceID) override {
        const char* lvl = "info";
        switch (level) {
        case QWebEnginePage::InfoMessageLevel: lvl = "info"; break;
        case QWebEnginePage::WarningMessageLevel: lvl = "warn"; break;
        case QWebEnginePage::ErrorMessageLevel: lvl = "error"; break;
        }
        qWarning().nospace() << "[WebEngine JS " << lvl << "] "
                             << sourceID << ':' << lineNumber << ": " << message;
        QWebEnginePage::javaScriptConsoleMessage(level, message, lineNumber, sourceID);
    }
#else
    void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level,
                                  const QString& message,
                                  int lineNumber,
                                  const QString& sourceID) override {
        Q_UNUSED(level);
        qWarning().nospace() << "[WebEngine JS] "
                             << sourceID << ':' << lineNumber << ": " << message;
        QWebEnginePage::javaScriptConsoleMessage(level, message, lineNumber, sourceID);
    }
#endif
};
#else
// Fallback stub when Qt WebEngine is not available; keeps build happy even if
// headers are included conditionally in sources.
class WebPage : public QObject {
public:
    using QObject::QObject;
    virtual ~WebPage() = default;
};
#endif
