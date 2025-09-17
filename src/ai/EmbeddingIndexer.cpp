#include "ai/EmbeddingIndexer.h"

#include <QFileInfo>
#include <QDir>
#include <QCryptographicHash>
#include <QProcess>
#include <QPdfDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QStandardPaths>
#include <QUuid>
#include <QThread>

EmbeddingIndexer::EmbeddingIndexer(const Params& p, QObject* parent)
    : QObject(parent), p_(p) {}

QStringList EmbeddingIndexer::extractPagesText() {
    QStringList pages;
    QPdfDocument doc;
    auto st = doc.load(p_.pdfPath);
    if (st != static_cast<QPdfDocument::Error>(0)) {
        emit warn(tr("Falha ao abrir PDF para contagem de páginas."));
        return pages;
    }
    const int pageCount = doc.pageCount();
    emit metric(QStringLiteral("pages"), QString::number(pageCount));

    const bool hasPdfToText = !QStandardPaths::findExecutable("pdftotext").isEmpty();
    const bool hasPdfToPpm = !QStandardPaths::findExecutable("pdftoppm").isEmpty();
    const bool hasTesseract = !QStandardPaths::findExecutable("tesseract").isEmpty();

    if (!hasPdfToText) {
        emit warn(tr("Ferramenta 'pdftotext' não encontrada. Instale o pacote 'poppler-utils' para extração de texto mais rápida."));
    }
    if (!hasTesseract) {
        emit warn(tr("Ferramenta 'tesseract' não encontrada. Instale 'tesseract-ocr' para fallback via OCR."));
    }

    // Preferir pdftotext; fallback para pdftoppm+tesseract; por fim páginas vazias
    for (int i = 1; i <= pageCount; ++i) {
        if (QThread::currentThread()->isInterruptionRequested()) break;
        QString pageText;
        if (hasPdfToText) {
            QProcess proc;
            QStringList args; args << "-q" << "-layout" << "-eol" << "unix" << "-f" << QString::number(i) << "-l" << QString::number(i) << p_.pdfPath << "-";
            proc.start("pdftotext", args);
            if (proc.waitForStarted(2000)) {
                proc.waitForFinished(20000);
                pageText = QString::fromUtf8(proc.readAllStandardOutput());
            }
        }
        if (pageText.trimmed().isEmpty() && hasPdfToPpm && hasTesseract) {
            // Fallback: renderiza a página como PNG e roda OCR
            const QString tmpDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
            QDir().mkpath(tmpDir);
            const QString base = QUuid::createUuid().toString(QUuid::WithoutBraces);
            const QString outPngBase = QDir(tmpDir).filePath(base);
            // pdftoppm -f i -l i -r 200 -png pdf outPngBase
            QProcess ppm;
            QStringList a; a << "-f" << QString::number(i) << "-l" << QString::number(i) << "-r" << "200" << "-png" << p_.pdfPath << outPngBase;
            ppm.start("pdftoppm", a);
            if (ppm.waitForStarted(2000)) {
                ppm.waitForFinished(30000);
                const QString imgPath = outPngBase + "-" + QString::number(i) + ".png";
                if (QFileInfo::exists(imgPath)) {
                    QProcess ocr;
                    QStringList ta; ta << imgPath << "stdout" << "-l" << "por+eng" << "--psm" << "6";
                    ocr.start("tesseract", ta);
                    if (ocr.waitForStarted(2000)) {
                        ocr.waitForFinished(30000);
                        pageText = QString::fromUtf8(ocr.readAllStandardOutput());
                    }
                    QFile::remove(imgPath);
                }
            }
        }
        if (pageText.isEmpty()) {
            if (!hasPdfToText && !(hasPdfToPpm && hasTesseract)) {
                emit warn(tr("Sem ferramentas de extração instaladas. Instale 'poppler-utils' (pdftotext) e/ou 'tesseract-ocr'."));
            }
        }
        pages << pageText;
    }
    return pages;
}

QStringList EmbeddingIndexer::chunkText(const QString& text, int chunkSize, int overlap) const {
    QString t = text;
    t.replace('\r', '\n');
    QStringList lines = t.split('\n');
    for (QString& l : lines) l = l.trimmed();
    t = lines.join('\n');
    QStringList chunks;
    int i = 0; const int n = t.size();
    if (n == 0) return chunks;
    while (i < n) {
        const int j = qMin(n, i + chunkSize);
        const QString c = t.mid(i, j - i).trimmed();
        if (!c.isEmpty()) chunks << c;
        i = j - overlap; if (i < 0) i = 0; if (i >= n) break;
    }
    return chunks;
}

QString EmbeddingIndexer::sha1(const QString& s) const {
    QCryptographicHash h(QCryptographicHash::Sha1);
    h.addData(s.toUtf8());
    return QString::fromLatin1(h.result().toHex());
}

void EmbeddingIndexer::run() {
    QElapsedTimer total; total.start();
    emit stage(tr("Lendo PDF"));
    const QStringList pages = extractPagesText();
    const int pageCount = pages.size();
    if (pageCount == 0) { emit error(tr("Sem páginas extraídas.")); emit finished(false, tr("Falha na extração de texto")); return; }

    // Passo 1: contar chunks sem acumular em memória
    emit stage(tr("Contando chunks"));
    qint64 totalChunks = 0;
    for (int i=0;i<pageCount;++i) {
        if (QThread::currentThread()->isInterruptionRequested()) { emit warn(tr("Interrompido")); emit finished(false, tr("Cancelado")); return; }
        const QStringList cs = chunkText(pages[i], p_.chunkSize, p_.chunkOverlap);
        totalChunks += cs.size();
    }
    emit metric(QStringLiteral("chunks"), QString::number(totalChunks));
    if (totalChunks == 0) { emit warn(tr("Nenhum chunk gerado.")); emit finished(true, tr("Nada para indexar")); return; }

    // Preparar arquivos de saída (escrita incremental)
    emit stage(tr("Preparando arquivos"));
    QDir().mkpath(p_.dbDir);
    const QString fileHash = sha1(QFileInfo(p_.pdfPath).absoluteFilePath());
    const QString base = QStringLiteral("%1/index_%2_%3").arg(p_.dbDir, fileHash, p_.providerCfg.model).replace(':','_').replace('/','_');
    const QString binPath = base + ".bin";
    const QString idsPath = base + ".ids.json";
    const QString metaPath = base + ".meta.json";

    QFile fb(binPath);
    if (!fb.open(QIODevice::WriteOnly)) { emit error(tr("Falha ao salvar binário de vetores")); emit finished(false, tr("Falha ao abrir binário")); return; }
    // Header temporário: magic + count + dim(0)
    fb.write("VEC1", 4);
    int countInt = int(totalChunks);
    int dimInt = 0; // desconhecido até a 1ª resposta
    qint64 posCount = fb.pos();
    fb.write(reinterpret_cast<const char*>(&countInt), sizeof(int));
    qint64 posDim = fb.pos();
    fb.write(reinterpret_cast<const char*>(&dimInt), sizeof(int));

    QFile fids(idsPath);
    if (!fids.open(QIODevice::WriteOnly)) { emit error(tr("Falha ao salvar JSON de ids")); fb.close(); QFile::remove(binPath); emit finished(false, tr("Falha ao abrir ids")); return; }
    fids.write("["); bool firstId = true;

    QFile fmeta(metaPath);
    if (!fmeta.open(QIODevice::WriteOnly)) { emit error(tr("Falha ao salvar metadados")); fb.close(); fids.close(); QFile::remove(binPath); QFile::remove(idsPath); emit finished(false, tr("Falha ao abrir meta")); return; }
    fmeta.write("["); bool firstMeta = true;

    emit stage(tr("Gerando embeddings"));
    EmbeddingProvider prov(p_.providerCfg);
    const int bs = qMax(1, p_.batchSize);
    qint64 processed = 0;
    int globalChunkIdx = 0;
    int pagesProcessed = 0;
    for (int i=0;i<pageCount;++i) {
        if (QThread::currentThread()->isInterruptionRequested()) { emit warn(tr("Interrompido")); break; }
        const QStringList cs = chunkText(pages[i], p_.chunkSize, p_.chunkOverlap);
        QStringList batchTexts; batchTexts.reserve(bs);
        QList<QPair<int,int>> batchLocs; batchLocs.reserve(bs);
        for (int j=0;j<cs.size();++j) {
            batchTexts << cs[j];
            batchLocs << QPair<int,int>(i+1, j);
            if (batchTexts.size() == bs || (j+1)==cs.size()) {
                // Embedding do lote
                QList<QVector<float>> vecs;
                try { vecs = prov.embedBatch(batchTexts); }
                catch (const std::exception& ex) { emit error(QString::fromUtf8(ex.what())); fb.close(); fids.close(); fmeta.close(); emit finished(false, tr("Falha ao gerar embeddings")); return; }
                // Inicializar dim na 1ª vez
                if (dimInt == 0 && !vecs.isEmpty()) {
                    dimInt = vecs.first().size();
                    // escrever dim real no header
                    qint64 cur = fb.pos(); fb.seek(posDim); fb.write(reinterpret_cast<const char*>(&dimInt), sizeof(int)); fb.seek(cur);
                }
                // Persistir vetores/ids/meta
                for (int k=0;k<vecs.size();++k) {
                    const QVector<float>& v = vecs[k];
                    fb.write(reinterpret_cast<const char*>(v.data()), sizeof(float)*v.size());
                    // id
                    if (!firstId) fids.write(","); firstId = false;
                    fids.write("\""); fids.write(QString::number(globalChunkIdx).toUtf8()); fids.write("\"");
                    // meta
                    const auto& loc = batchLocs[k];
                    QJsonObject o; o.insert("file", QFileInfo(p_.pdfPath).absoluteFilePath()); o.insert("page", loc.first); o.insert("chunk", loc.second); o.insert("model", p_.providerCfg.model); o.insert("provider", p_.providerCfg.provider);
                    if (!firstMeta) fmeta.write(","); firstMeta = false;
                    fmeta.write(QJsonDocument(o).toJson(QJsonDocument::Compact));
                    ++globalChunkIdx;
                }
                processed += batchTexts.size();
                const int pct = int((double(processed) / double(totalChunks)) * 100.0);
                emit progress(pct, tr("embedding batch size=%1 (página %2)").arg(batchTexts.size()).arg(i+1));
                if (p_.pauseMsBetweenBatches > 0) {
                    QThread::msleep(static_cast<unsigned long>(p_.pauseMsBetweenBatches));
                }
                batchTexts.clear(); batchLocs.clear();
            }
        }
        ++pagesProcessed;
        if (p_.pagesPerStage > 0 && pagesProcessed >= p_.pagesPerStage) {
            // Concluir etapa sem processar o documento inteiro, para evitar exaustão
            break;
        }
    }

    // Finalizar arquivos
    fids.write("]"); fids.close();
    fmeta.write("]"); fmeta.close();
    // Garantir que count e dim estejam corretos
    fb.seek(posCount); countInt = globalChunkIdx; fb.write(reinterpret_cast<const char*>(&countInt), sizeof(int));
    if (dimInt == 0) { dimInt = 1; }
    fb.seek(posDim); fb.write(reinterpret_cast<const char*>(&dimInt), sizeof(int));
    fb.close();

    if (processed == 0) { emit warn(tr("Nenhum vetor persistido.")); emit finished(false, tr("Nada produzido")); return; }

    emit stage(tr("Concluído"));
    emit metric(QStringLiteral("total_time_ms"), QString::number(total.elapsed()));
    if (p_.pagesPerStage > 0 && pagesProcessed >= p_.pagesPerStage && pagesProcessed < pageCount) {
        const int pctDoc = int((double(pagesProcessed) / double(pageCount)) * 100.0);
        emit progress(pctDoc, tr("etapa concluída: %1/%2 páginas").arg(pagesProcessed).arg(pageCount));
        emit finished(true, tr("Etapa concluída (%1 páginas). Execute novamente para continuar.").arg(pagesProcessed));
        return;
    }
    emit progress(100, QStringLiteral("completed"));
    emit finished(true, tr("Indexação concluída"));
}
