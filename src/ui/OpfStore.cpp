#include "ui/OpfStore.h"

#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QSettings>
#include <QCryptographicHash>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

QString OpfStore::defaultOpfPathFor(const QString& bookPath) {
    // Use cache directory with configurable base path: opf/db_path
    QSettings s;
    const QString defaultCache = QDir(QDir::home().filePath(".cache")).filePath("br.tec.rapport.genai-reader/opf");
    const QString opfDir = s.value("opf/db_path", defaultCache).toString();

    // Build a stable hashed filename derived from absolute PDF path
    const QString abs = QFileInfo(bookPath).absoluteFilePath();
    QCryptographicHash h(QCryptographicHash::Sha1);
    h.addData(abs.toUtf8());
    const QString hex = QString::fromLatin1(h.result().toHex());

    const QString baseName = QFileInfo(bookPath).completeBaseName();
    const QString fileName = QStringLiteral("%1_%2.opf").arg(baseName, hex.left(12));
    return QDir(opfDir).filePath(fileName);
}

static QString readTextElem(QXmlStreamReader& xr) {
    QString out;
    while (!xr.atEnd()) {
        xr.readNext();
        if (xr.isCharacters()) out += xr.text().toString();
        else if (xr.isEndElement()) break;
    }
    return out.trimmed();
}

bool OpfStore::read(const QString& opfPath, OpfData* out, QString* errorMsg) {
    if (!out) return false;
    QFile f(opfPath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (errorMsg) *errorMsg = QStringLiteral("Falha ao abrir OPF: %1").arg(opfPath);
        return false;
    }
    QXmlStreamReader xr(&f);
    OpfData data;
    while (!xr.atEnd()) {
        xr.readNext();
        if (xr.isStartElement()) {
            const QString name = xr.name().toString();
            if (name == QLatin1String("dc:title")) data.title = readTextElem(xr);
            else if (name == QLatin1String("dc:creator")) data.author = readTextElem(xr);
            else if (name == QLatin1String("dc:publisher")) data.publisher = readTextElem(xr);
            else if (name == QLatin1String("dc:language")) data.language = readTextElem(xr);
            else if (name == QLatin1String("dc:identifier")) data.identifier = readTextElem(xr);
            else if (name == QLatin1String("dc:description")) data.description = readTextElem(xr);
            else if (name == QLatin1String("meta")) {
                const auto attrs = xr.attributes();
                const QString nameAttr = attrs.value("name").toString();
                if (nameAttr == QLatin1String("summary")) {
                    data.summary = attrs.value("content").toString();
                } else if (nameAttr == QLatin1String("keywords")) {
                    data.keywords = attrs.value("content").toString();
                }
            }
        }
    }
    if (xr.hasError()) {
        if (errorMsg) *errorMsg = QStringLiteral("Erro ao ler OPF: %1").arg(xr.errorString());
        return false;
    }
    *out = data;
    return true;
}

bool OpfStore::write(const QString& opfPath, const OpfData& d, QString* errorMsg) {
    QDir().mkpath(QFileInfo(opfPath).absolutePath());
    QFile f(opfPath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        if (errorMsg) *errorMsg = QStringLiteral("Falha ao escrever OPF: %1").arg(opfPath);
        return false;
    }
    QXmlStreamWriter xw(&f);
    xw.setAutoFormatting(true);
    xw.writeStartDocument();
    xw.writeStartElement("package");
    xw.writeAttribute("xmlns", "http://www.idpf.org/2007/opf");
    xw.writeAttribute("version", "3.0");

    xw.writeStartElement("metadata");
    xw.writeNamespace("http://purl.org/dc/elements/1.1/", "dc");

    if (!d.title.isEmpty()) xw.writeTextElement("dc:title", d.title);
    if (!d.author.isEmpty()) xw.writeTextElement("dc:creator", d.author);
    if (!d.publisher.isEmpty()) xw.writeTextElement("dc:publisher", d.publisher);
    if (!d.language.isEmpty()) xw.writeTextElement("dc:language", d.language);
    if (!d.identifier.isEmpty()) xw.writeTextElement("dc:identifier", d.identifier);
    if (!d.description.isEmpty()) xw.writeTextElement("dc:description", d.description);
    if (!d.summary.isEmpty()) {
        xw.writeEmptyElement("meta");
        xw.writeAttribute("name", "summary");
        xw.writeAttribute("content", d.summary);
    }
    if (!d.keywords.isEmpty()) {
        xw.writeEmptyElement("meta");
        xw.writeAttribute("name", "keywords");
        xw.writeAttribute("content", d.keywords);
    }

    xw.writeEndElement(); // metadata

    xw.writeStartElement("manifest");
    xw.writeEndElement();
    xw.writeStartElement("spine");
    xw.writeEndElement();

    xw.writeEndElement(); // package
    xw.writeEndDocument();
    f.close();
    return true;
}
