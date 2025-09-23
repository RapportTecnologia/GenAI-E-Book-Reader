#pragma once

#include <QString>
#include <QStringList>
#include <QMap>

struct OpfData {
    QString title;
    QString author;
    QString publisher;
    QString language;
    QString identifier; // e.g., ISBN or hash
    QString description; // long description
    QString summary;     // executive summary
    QString keywords;    // comma-separated
    // Extended fields (optional)
    QString edition;     // e.g., Second Edition
    QString source;      // e.g., origin site (WeLib.org, dokumen.pub)
    QString format;      // e.g., PDF, DJVU (may map to dc:format)
    QString isbn;        // canonical ISBN if available
};

class OpfStore {
public:
    // Returns default OPF file path given a book path (same dir, .opf extension)
    static QString defaultOpfPathFor(const QString& bookPath);

    // Read OPF file into OpfData. Returns true on success.
    static bool read(const QString& opfPath, OpfData* out, QString* errorMsg = nullptr);

    // Write OpfData to OPF file (creating directories if needed). Returns true on success.
    static bool write(const QString& opfPath, const OpfData& data, QString* errorMsg = nullptr);
};
