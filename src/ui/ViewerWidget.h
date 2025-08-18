#pragma once

#ifdef USE_QT
#include <QWidget>
#include <QString>

class ViewerWidget : public QWidget {
    Q_OBJECT
public:
    explicit ViewerWidget(QWidget* parent = nullptr);

    void setTotalPages(unsigned int total);
    void setCurrentPage(unsigned int page);
    void setZoomFactor(double factor);

    unsigned int totalPages() const { return totalPages_; }
    unsigned int currentPage() const { return currentPage_; }
    double zoomFactor() const { return zoomFactor_; }

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    unsigned int totalPages_ {0};
    unsigned int currentPage_ {0};
    double zoomFactor_ {1.0};
};
#endif // USE_QT
