/**
 * \file PdfViewerWidget.h
 * \brief Widget de visualização de PDF com navegação, zoom e seleção.
 *
 * Este widget encapsula a renderização do PDF usando QPdfDocument/QPdfView e
 * oferece utilidades de navegação, zoom, seleção de texto/retângulo e atalhos para
 * ações inteligentes (sinônimos, sumarização, envio ao chat, etc.).
 * \ingroup ui
 */

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
class QPrinter;

/**
 * \class PdfViewerWidget
 * \brief Visualizador de PDF com seleção de texto e retângulo.
 *
 * Fornece métodos simples para abrir um arquivo, navegar entre páginas (anterior/próxima),
 * ajustar zoom e recuperar texto selecionado. Também emite sinais para integração com a UI
 * e com o módulo de IA.
 * \ingroup ui
 */
class PdfViewerWidget : public QWidget {
    Q_OBJECT
public:
    explicit PdfViewerWidget(QWidget* parent = nullptr);
    ~PdfViewerWidget() override;

    /** \brief Abre um PDF pelo caminho. Retorna false e preenche \p errorOut em caso de falha. */
    bool openFile(const QString& path, QString* errorOut = nullptr);

    /** \brief Define o fator de zoom atual. */
    void setZoomFactor(double factor);
    /** \brief Obtém o fator de zoom atual. */
    double zoomFactor() const;

    /** \brief Vai para a próxima página. */
    void nextPage();
    /** \brief Volta para a página anterior. */
    void prevPage();

    /** \brief Número total de páginas do documento atual. */
    unsigned int totalPages() const;
    /** \brief Página atual (baseada em 1). */
    unsigned int currentPage() const;
    /** \brief Define a página atual (1..totalPages). */
    void setCurrentPage(unsigned int page);

    QPdfDocument* document() const { return doc_; }
    /** \brief Caminho absoluto do arquivo PDF atualmente aberto (se houver). */
    QString currentFilePath() const { return filePath_; }

    // Convenience: fit content to widget width for natural reading
    /** \brief Ajusta o PDF para caber na largura do widget. */
    void fitToWidth();

    /** \brief Imprime a página atual (conteúdo visível da página). */
    void printCurrentPage();

    enum class SelectionMode { Auto, Text, Rect, None };
    /** \brief Define o modo de seleção (automático, texto, retângulo ou nenhum). */
    void setSelectionMode(SelectionMode mode);
    SelectionMode selectionMode() const { return selMode_; }
    /** \brief Indica se existe uma seleção ativa. */
    bool hasSelection() const;
    /** \brief Limpa a seleção atual (texto e/ou retângulo). */
    void clearSelection();
    /** \brief Copia o conteúdo da seleção (texto ou OCR) para a área de transferência. */
    void copySelection();
    /** \brief Salva a seleção como .txt. */
    void saveSelectionAsTxt();
    /** \brief Salva a seleção como .md (Markdown). */
    void saveSelectionAsMarkdown();

    // OCR: when text selection API is not available or selection spans multiple pages,
    // allow extracting text via Tesseract if present in PATH.
    /** \brief Aplica OCR à seleção retangular. Retorna o texto e preenche \p ok. */
    QString ocrSelectionText(bool* ok = nullptr);

    // Return current selection as text. If text selection is empty and a rect exists,
    // try OCR when possible.
    /** \brief Retorna o texto da seleção; usa OCR quando necessário. */
    QString selectionText(bool* ok = nullptr);

    // Preferences
    /** \brief Define o passo de zoom aplicado pela roda do mouse. */
    void setWheelZoomStep(double step);
    // Scroll helpers
    /** \brief Valor do scroll vertical atual. */
    int verticalScrollValue() const;
    /** \brief Define o scroll vertical. */
    void setVerticalScrollValue(int v);

    // Visual hint for search hits
    /** \brief Efeito visual temporário para indicar destaque de busca. */
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
    QString filePath_;

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
    /**
     * \brief Emite quando o fator de zoom muda.
     * \param factor Fator de zoom atual.
     */
    void zoomFactorChanged(double factor);

    /**
     * \brief Emite quando o scroll vertical muda.
     * \param value Valor do scroll vertical atual.
     */
    void scrollChanged(int value);

    /**
     * \brief Emite quando a página atual muda.
     * \param page Número da página atual.
     */
    void pageChanged(int page);

    // Emitted when user requests actions from context menu
    /**
     * \brief Solicita sinônimos para o termo selecionado.
     * \param wordOrLocution Termo selecionado.
     */
    void requestSynonyms(const QString& wordOrLocution);

    /**
     * \brief Solicita sumarização do texto selecionado.
     * \param text Texto selecionado.
     */
    void requestSummarize(const QString& text);

    /**
     * \brief Solicita envio do texto selecionado para o chat.
     * \param text Texto selecionado.
     */
    void requestSendToChat(const QString& text);

    /**
     * \brief Solicita envio de imagem (seleção) para o chat.
     * \param image Imagem selecionada.
     */
    void requestSendImageToChat(const QImage& image);

    /**
     * \brief Solicita consulta de dicionário para o termo.
     * \param term Termo selecionado.
     */
    void requestDictionaryLookup(const QString& term);

    /**
     * \brief Solicita reconstrução das embeddings do documento.
     */
    void requestRebuildEmbeddings();

    /**
     * \brief Solicita sumarização de todo o documento.
     */
    void requestSummarizeDocument();
};
