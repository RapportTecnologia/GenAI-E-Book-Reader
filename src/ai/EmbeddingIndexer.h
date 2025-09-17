#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QStringView>
#include <QVector>
#include <QElapsedTimer>
#include <QMutex>
#include <QWaitCondition>
#include <functional>

#include "ai/EmbeddingProvider.h"
#include "ai/VectorIndex.h"

class EmbeddingIndexer : public QObject {
    Q_OBJECT
public:
    struct Params {
        QString pdfPath;
        QString dbDir; // base dir ~/.cache/br.tec.rapport.genai-reader
        EmbeddingProvider::Config providerCfg;
        int chunkSize {1000};
        int chunkOverlap {200};
        int batchSize {16};
        int pagesPerStage {-1}; // <=0 means all pages
        int pauseMsBetweenBatches {0}; // simple throttle to avoid resource exhaustion
    };

    explicit EmbeddingIndexer(const Params& p, QObject* parent = nullptr);

signals:
    void stage(const QString& name);
    void metric(const QString& key, const QString& value);
    void progress(int pct, const QString& info);
    void warn(const QString& m);
    void error(const QString& m);
    void finished(bool ok, const QString& message);

public slots:
    void run();
    // Control from UI
    void requestPause();
    void requestResume();

private:
    QStringList extractPagesText(); // tries pdftotext page-by-page; fallback returns empty pages
    void forEachChunks(const QString& text, int chunkSize, int overlap,
                       const std::function<bool(QStringView, int)>& consume);
    QString sha1(const QString& s) const;
    void waitIfPaused();

private:
    Params p_;
    // Pause control
    QMutex pauseMutex_;
    QWaitCondition pauseCond_;
    bool paused_ {false};
};

