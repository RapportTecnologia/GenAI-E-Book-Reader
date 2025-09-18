#pragma once

#include <QVector>
#include <QList>
#include <QString>
#include <QByteArray>
#include <QFile>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QtMath>

// Minimal cosine-similarity index (linear scan). Stores vectors and metadata ids.
class VectorIndex {
public:
    enum class Metric { Cosine, Dot, L2 };
    void setVectors(const QList<QVector<float>>& vecs) { vecs_ = vecs; }
    void setIds(const QStringList& ids) { ids_ = ids; }

    const QList<QVector<float>>& vectors() const { return vecs_; }
    const QStringList& ids() const { return ids_; }

    // Save vectors to a binary file and ids to JSON for simplicity
    bool save(const QString& binPath, const QString& idsJsonPath, QString* err = nullptr) const {
        QFile fb(binPath);
        if (!fb.open(QIODevice::WriteOnly)) { if (err) *err = QObject::tr("Falha ao salvar binário de vetores"); return false; }
        // header: magic, count, dim
        if (vecs_.isEmpty()) { fb.close(); return true; }
        const int count = vecs_.size();
        const int dim = vecs_.first().size();
        fb.write("VEC1", 4);
        fb.write(reinterpret_cast<const char*>(&count), sizeof(int));
        fb.write(reinterpret_cast<const char*>(&dim), sizeof(int));
        for (const auto& v : vecs_) {
            fb.write(reinterpret_cast<const char*>(v.data()), sizeof(float)*dim);
        }
        fb.close();

        // ids
        QJsonArray arr; for (const auto& id : ids_) arr.append(id);
        QFile fj(idsJsonPath); if (!fj.open(QIODevice::WriteOnly)) { if (err) *err = QObject::tr("Falha ao salvar JSON de ids"); return false; }
        fj.write(QJsonDocument(arr).toJson(QJsonDocument::Compact)); fj.close();
        return true;
    }

    bool load(const QString& binPath, const QString& idsJsonPath, QString* err = nullptr) {
        vecs_.clear(); ids_.clear();
        QFile fb(binPath);
        if (!fb.open(QIODevice::ReadOnly)) { if (err) *err = QObject::tr("Falha ao abrir binário de vetores"); return false; }
        char magic[4]; fb.read(magic, 4);
        int count=0, dim=0; fb.read(reinterpret_cast<char*>(&count), sizeof(int)); fb.read(reinterpret_cast<char*>(&dim), sizeof(int));
        if (strncmp(magic, "VEC1", 4) != 0 || count < 0 || dim <= 0) { if (err) *err = QObject::tr("Arquivo de vetores inválido"); fb.close(); return false; }
        vecs_.reserve(count);
        for (int i=0;i<count;++i){ QVector<float> v; v.resize(dim); fb.read(reinterpret_cast<char*>(v.data()), sizeof(float)*dim); vecs_.append(v);} fb.close();
        QFile fj(idsJsonPath); if (!fj.open(QIODevice::ReadOnly)) { if (err) *err = QObject::tr("Falha ao abrir JSON de ids"); return false; }
        const QJsonDocument doc = QJsonDocument::fromJson(fj.readAll()); fj.close();
        for (const auto& x : doc.array()) ids_.append(x.toString());
        return ids_.size() == vecs_.size();
    }

    struct Hit { int index; float score; };

    // Backward-compatible: defaults to cosine similarity
    QList<Hit> topK(const QVector<float>& query, int k) const {
        return topK(query, k, Metric::Cosine);
    }

    QList<Hit> topK(const QVector<float>& query, int k, Metric metric) const {
        QList<Hit> hits;
        if (vecs_.isEmpty()) return hits;
        const int dim = vecs_.first().size();
        auto dot = [&](const QVector<float>& a, const QVector<float>& b){ double s=0; for(int i=0;i<dim;++i) s += double(a[i])*double(b[i]); return s; };
        auto norm = [&](const QVector<float>& a){ double s=0; for(float v: a) s += double(v)*double(v); return std::sqrt(s); };
        const double nq = norm(query);
        for (int i=0;i<vecs_.size();++i){
            double score = 0.0;
            if (metric == Metric::Cosine) {
                const double nv = norm(vecs_[i]);
                const double d = dot(query, vecs_[i]);
                score = (nq>0 && nv>0) ? (d/(nq*nv)) : 0.0;
            } else if (metric == Metric::Dot) {
                score = dot(query, vecs_[i]);
            } else { // L2: use negative distance so that higher is better
                double s=0; for (int j=0;j<dim;++j){ const double dv = double(query[j]) - double(vecs_[i][j]); s += dv*dv; }
                score = -std::sqrt(s);
            }
            hits.append({i, float(score)});
        }
        std::sort(hits.begin(), hits.end(), [](const Hit& a, const Hit& b){ return a.score > b.score; });
        if (k < hits.size()) hits = hits.mid(0, k);
        return hits;
    }

private:
    QList<QVector<float>> vecs_;
    QStringList ids_;
};
