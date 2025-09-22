 #pragma once

#ifdef HAVE_QT_WEBENGINE
#include <QWebEngineProfile>
#include <QStandardPaths>
#include <QDir>

// Returns a shared off-the-record profile configured to minimize disk usage
// and the number of open file descriptors. Use this for lightweight views
// like chat history and tutorial rendering where persistence is unnecessary.
inline QWebEngineProfile* sharedEphemeralWebProfile() {
    static QWebEngineProfile* s = []{
        auto* p = new QWebEngineProfile();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        p->setOffTheRecord(true); // Qt5 API
#else
        // Qt6: profile is off-the-record if no persistent path is set (default)
        // Ensure we do not set any persistent paths.
#endif
        p->setHttpCacheType(QWebEngineProfile::MemoryHttpCache);
        p->setPersistentCookiesPolicy(QWebEngineProfile::NoPersistentCookies);
        // Disable spellcheck and other features that may open files
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
        p->setSpellCheckEnabled(false);
#endif
        return p;
    }();
    return s;
}
#else
// Stub for builds without WebEngine available
inline void* sharedEphemeralWebProfile() { return nullptr; }
#endif
