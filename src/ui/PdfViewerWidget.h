#pragma once

#ifdef USE_QT
#ifdef HAVE_QT_PDF
#include <QWidget>
#include <QPdfDocument>
#include <QPdfView>
#include <QPdfPageNavigator>

class PdfViewerWidget : public QWidget {
    Q_OBJECT
public:
    explicit PdfViewerWidget(QWidget* parent = nullptr);
    ~PdfViewerWidget() override;

    bool openFile(const QString& path, QString* errorOut = nullptr);

    void setZoomFactor(double factor);
    double zoomFactor() const;

    void nextPage();
    void prevPage();

    unsigned int totalPages() const;
    unsigned int currentPage() const;
    void setCurrentPage(unsigned int page);

    QPdfDocument* document() const { return doc_; }

private:
    QPdfDocument* doc_;
    QPdfView* view_;
    QPdfPageNavigator* navigation_;
};
#endif // HAVE_QT_PDF
#endif // USE_QT
