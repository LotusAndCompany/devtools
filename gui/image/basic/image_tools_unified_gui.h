#ifndef IMAGE_TOOLS_UNIFIED_GUI_H
#define IMAGE_TOOLS_UNIFIED_GUI_H

#include "gui/gui_tool.h"

#include <QColor>
#include <QImage>
#include <QWidget>

#include <cstdint>

class QAbstractButton;
class QButtonGroup;
class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QFrame;
class QLabel;
class QPushButton;
class QRadioButton;
class QScrollArea;
class QSpinBox;
class BasicImageViewControl;
class ColorSample;
class ImageViewForImageTransparent;

namespace Ui {
/**
 * @brief ImageToolsUnifiedGUI が保持するウィジェット群
 */
struct ImageToolsUnifiedGUI // NOLINT(altera-struct-pack-align)
{
    ImageViewForImageTransparent *imageView;
    QScrollArea *toolScrollArea;
    BasicImageViewControl *control;

    QFrame *resizeSection;
    QFrame *transformSection;
    QFrame *transparentSection;
    QFrame *divisionSection;
    QCheckBox *resizeSectionToggle;
    QCheckBox *transformSectionToggle;
    QCheckBox *transparentSectionToggle;
    QCheckBox *divisionSectionToggle;
    QWidget *resizeSectionBody;
    QWidget *transformSectionBody;
    QWidget *transparentSectionBody;
    QWidget *divisionSectionBody;

    QSpinBox *widthValue;
    QSpinBox *heightValue;
    QDoubleSpinBox *hScaleValue;
    QDoubleSpinBox *vScaleValue;
    QCheckBox *keepAspectRatio;
    QCheckBox *smoothScaling;
    QPushButton *applyResizeBySizeButton;
    QPushButton *applyResizeByScaleButton;

    QPushButton *rotateLeftButton;
    QPushButton *rotateRightButton;
    QPushButton *flipHorizontalButton;
    QPushButton *flipVerticalButton;

    QComboBox *colorMode;
    ColorSample *colorSample;
    QDoubleSpinBox *toleranceValue;
    QDoubleSpinBox *transparencyValue;
    QCheckBox *contiguousArea;

    QLabel *sizeLabel;
    QRadioButton *useDivisionButton;
    QRadioButton *useSizeButton;
    QButtonGroup *divisionModeButtonGroup;
    QSpinBox *hDivValue;
    QSpinBox *vDivValue;
    QSpinBox *cellWidthValue;
    QSpinBox *cellHeightValue;
    QCheckBox *ignoreRemainders;
    QPushButton *saveDividedButton;
};
} // namespace Ui

/**
 * @brief 画像編集の主要4ツールを1画面に統合したGUI
 */
class ImageToolsUnifiedGUI : public GuiTool
{
    Q_OBJECT

public:
    /**
     * @brief コンストラクタ
     * @param parent 親ウィジェット
     */
    explicit ImageToolsUnifiedGUI(QWidget *parent = nullptr);
    ~ImageToolsUnifiedGUI() override;
    ImageToolsUnifiedGUI(const ImageToolsUnifiedGUI &) = delete;
    ImageToolsUnifiedGUI &operator=(const ImageToolsUnifiedGUI &) = delete;
    ImageToolsUnifiedGUI(ImageToolsUnifiedGUI &&) = delete;
    ImageToolsUnifiedGUI &operator=(ImageToolsUnifiedGUI &&) = delete;

protected:
    void changeEvent(QEvent *event) override;

private slots:
    void onLoadImageSelected(const QString &path);
    void onSaveImageSelected(const QString &path);
    void onResetButtonClicked();

    void onApplyResizeBySizeClicked();
    void onApplyResizeByScaleClicked();
    void onKeepAspectRatioChanged(Qt::CheckState state);

    void onRotateLeftButtonClicked();
    void onRotateRightButtonClicked();
    void onFlipHorizontalButtonClicked();
    void onFlipVerticalButtonClicked();

    void onColorModeIndexChanged(int index);
    void onPixelSelected(const QPoint &point, const QColor &color);
    void onToleranceValueChanged(double tolerance);
    void onTransparencyValueChanged(double transparency);
    void onContiguousAreaCheckStateChanged(Qt::CheckState state);

    void onDivisionModeClicked(QAbstractButton *button);
    void onSaveDividedButtonClicked();
    void onSectionVisibilityChanged();

private:
    enum class DivisionMode : uint8_t {
        DIVISION,
        CELL_SIZE,
    };

    Ui::ImageToolsUnifiedGUI *const ui;

    QString loadedFilePath;
    QImage originalImage;
    QImage currentImage;

    bool keepAspectRatio = false;
    bool onlyContiguousArea = true;
    QColor::Spec transparentColorSpec = QColor::Spec::Rgb;
    double transparentTolerance = 0.1;
    uint8_t transparentOpacity = 0;
    DivisionMode divisionMode = DivisionMode::DIVISION;

    void refreshPreview(bool resetScale = false);
    void refreshSizeInputs();
    void applyResize(const QSize &targetSize);
    bool hasImage() const;
    void retranslateUi();
    void retranslateSectionToggles();

    static double colorDiffSquaredRgb(const QColor &a, const QColor &b);
    static double colorDiffSquaredHsv(const QColor &a, const QColor &b);
    static double colorDiffSquaredHsl(const QColor &a, const QColor &b);
    static double maxColorDiffSquared(QColor::Spec spec);
    static bool isTransparentTarget(const QColor &source, const QColor &target, QColor::Spec spec,
                                    double tolerance);

    void applyTransparentByColor(const QColor &targetColor);
    void applyTransparentByFloodFill(const QPoint &start);
    void applyTransparencyAt(const QPoint &point, const QColor &color);

    bool saveDividedImages(const QString &folderPath) const;
    static int countByCellSize(int source, int cell, bool ignoreRemainders);
    QString outputSuffix() const;
};

#endif // IMAGE_TOOLS_UNIFIED_GUI_H
