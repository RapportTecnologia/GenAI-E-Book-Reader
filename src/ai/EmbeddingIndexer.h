#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QElapsedTimer>

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

private:
    QStringList extractPagesText(); // tries pdftotext page-by-page; fallback returns empty pages
    QStringList chunkText(const QString& text, int chunkSize, int overlap) const;
    QString sha1(const QString& s) const;

private:
    Params p_;
};
