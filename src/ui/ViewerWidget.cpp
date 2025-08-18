#ifdef USE_QT
#include "ui/ViewerWidget.h"
#include <QPainter>
#include <QPaintEvent>
#include <QFont>

ViewerWidget::ViewerWidget(QWidget* parent)
    : QWidget(parent) {
    setMinimumSize(400, 300);
}

void ViewerWidget::setTotalPages(unsigned int total) {
    totalPages_ = total;
    update();
}

void ViewerWidget::setCurrentPage(unsigned int page) {
    currentPage_ = page;
    update();
}

void ViewerWidget::setZoomFactor(double factor) {
    zoomFactor_ = factor;
    if (zoomFactor_ < 0.25) zoomFactor_ = 0.25;
    if (zoomFactor_ > 4.0) zoomFactor_ = 4.0;
    update();
}

void ViewerWidget::paintEvent(QPaintEvent* event) {
    QWidget::paintEvent(event);
    QPainter p(this);

    // Background
    p.fillRect(rect(), palette().base());

    // Draw placeholder page rectangle depending on zoom
    const int margin = 20;
    QSize pageSize = QSize(int((width() - 2*margin) * 0.8 * zoomFactor_), int((height() - 2*margin) * 0.8 * zoomFactor_));
    pageSize.setWidth(std::min(pageSize.width(), width() - 2*margin));
    pageSize.setHeight(std::min(pageSize.height(), height() - 2*margin));

    QRect pageRect(QPoint((width() - pageSize.width())/2, (height() - pageSize.height())/2), pageSize);
    p.setPen(Qt::gray);
    p.setBrush(palette().window());
    p.drawRect(pageRect);

    // Text: current page / total
    p.setPen(palette().text().color());
    QFont f = font();
    f.setPointSize(int(f.pointSize() * 1.2));
    p.setFont(f);
    QString info = QString("Page %1 / %2\nZoom: %3%")
                       .arg(currentPage_ == 0 ? 1 : currentPage_)
                       .arg(totalPages_ == 0 ? 100 : totalPages_)
                       .arg(int(zoomFactor_ * 100));
    p.drawText(pageRect, Qt::AlignCenter, info);
}
#endif // USE_QT
