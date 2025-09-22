#include <QWidget>
#include <QString>
#include <QImage>
#include <QPoint>
#include <QRect>
#include <QSize>
#include <QTimer>
#include <QVariantAnimation>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QEvent>
#include <QObject>
#include <QPdfDocument>
#include <QPdfView>
#include <QPdfPageNavigator>
#include <QRubberBand>
#include <QLabel>
#include <QGraphicsOpacityEffect>
#include <QMenu>

// Check if QPdfLinkModel is available (Qt 6.4+)
// For Qt 5.15, we don't have QPdfLinkModel, so we'll implement a different approach
class QGraphicsOpacityEffect;
class QMenu;

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

    // Convenience: fit content to widget width for natural reading
    void fitToWidth();

    enum class SelectionMode { Auto, Text, Rect, None };
    void setSelectionMode(SelectionMode mode);
    SelectionMode selectionMode() const { return selMode_; }
    bool hasSelection() const;
    void clearSelection();
    void copySelection();
    void saveSelectionAsTxt();
    void saveSelectionAsMarkdown();

    // OCR: when text selection API is not available or selection spans multiple pages,
    // allow extracting text via Tesseract if present in PATH.
    QString ocrSelectionText(bool* ok = nullptr);

    // Return current selection as text. If text selection is empty and a rect exists,
    // try OCR when possible.
    QString selectionText(bool* ok = nullptr);

    // Preferences
    void setWheelZoomStep(double step);
    // Scroll helpers
    int verticalScrollValue() const;
    void setVerticalScrollValue(int v);

    // Visual hint for search hits
    void flashHighlight();

private:
    void keyPressEvent(QKeyEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void setZoomAnimated(double target);
    void showToast(const QString& message);
    void showCopyToast();
    void startRectSelection(QMouseEvent* event, QObject* watched);
    QString extractTextFromSelectionNative();

    QPdfDocument* doc_;
    QPdfView* view_;
    QPdfPageNavigator* navigation_;

    // Selection state
    SelectionMode selMode_ { SelectionMode::Auto };
    bool selecting_ { false };
    QPoint selStart_;
    QRect selRect_;
    QRubberBand* rubber_ { nullptr };
    QString selectedText_;

    // Zoom wheel preferences and animation
    double wheelZoomStep_ { 1.1 };
    class QVariantAnimation* zoomAnim_ { nullptr };

    // Toast UI
    QLabel* toastLabel_ { nullptr };
    QGraphicsOpacityEffect* toastOpacity_ { nullptr };
    class QTimer* toastHideTimer_ { nullptr };

    // Temporary highlight overlay
    QLabel* highlightOverlay_ { nullptr };
    class QTimer* highlightTimer_ { nullptr };

signals:
    void zoomFactorChanged(double factor);
    void scrollChanged(int value);
    void pageChanged(int page);

    // Emitted when user requests actions from context menu
    void requestSynonyms(const QString& wordOrLocution);
    void requestSummarize(const QString& text);
    void requestSendToChat(const QString& text);
    void requestSendImageToChat(const QImage& image);
    void requestDictionaryLookup(const QString& term);
    void requestRebuildEmbeddings();
    void requestSummarizeDocument();
};
