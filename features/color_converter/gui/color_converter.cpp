#include "color_converter.h"

#include <QClipboard>
#include <QFont>
#include <QGridLayout>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QPixmap>
#include <QPushButton>
#include <QScreen>
#include <QToolButton>
#include <QVBoxLayout>

// ============================================================================
// SvPickerWidget
// ============================================================================

SvPickerWidget::SvPickerWidget(QWidget *parent) : QWidget(parent)
{
    setMinimumSize(160, 160);
    setCursor(Qt::CrossCursor);
}

void SvPickerWidget::setHue(qreal hue)
{
    m_hue = qBound(0.0, hue, 1.0);
    update();
}

void SvPickerWidget::setSaturationValue(qreal saturation, qreal value)
{
    m_saturation = qBound(0.0, saturation, 1.0);
    m_value = qBound(0.0, value, 1.0);
    update();
}

void SvPickerWidget::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const QRectF area = rect().adjusted(0, 0, -1, -1);

    // 1. 白 -> 純色(現在の色相) の水平グラデーション
    const QColor hueColor = QColor::fromHsvF(m_hue, 1.0, 1.0);
    QLinearGradient satGradient(area.topLeft(), area.topRight());
    satGradient.setColorAt(0.0, Qt::white);
    satGradient.setColorAt(1.0, hueColor);
    painter.fillRect(area, satGradient);

    // 2. 透明 -> 黒 の垂直グラデーションを重ねる
    QLinearGradient valGradient(area.topLeft(), area.bottomLeft());
    valGradient.setColorAt(0.0, Qt::transparent);
    valGradient.setColorAt(1.0, Qt::black);
    painter.fillRect(area, valGradient);

    // 枠線
    painter.setPen(QPen(palette().color(QPalette::Mid)));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(area);

    // 選択位置のインジケータ
    const qreal x = m_saturation * area.width();
    const qreal y = (1.0 - m_value) * area.height();
    const QPointF indicatorPos = area.topLeft() + QPointF(x, y);

    painter.setPen(QPen(Qt::white, 2));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(indicatorPos, 6, 6);
    painter.setPen(QPen(Qt::black, 1));
    painter.drawEllipse(indicatorPos, 7, 7);
}

void SvPickerWidget::mousePressEvent(QMouseEvent *event)
{
    updateFromPos(event->pos());
}

void SvPickerWidget::mouseMoveEvent(QMouseEvent *event)
{
    if ((event->buttons() & Qt::LeftButton) != 0) {
        updateFromPos(event->pos());
    }
}

void SvPickerWidget::updateFromPos(const QPoint &pos)
{
    const qreal w = qMax(1, width() - 1);
    const qreal h = qMax(1, height() - 1);

    m_saturation = qBound(0.0, pos.x() / w, 1.0);
    m_value = qBound(0.0, 1.0 - pos.y() / h, 1.0);

    update();
    emit saturationValueChanged(m_saturation, m_value);
}

// ============================================================================
// GradientSliderWidget
// ============================================================================

GradientSliderWidget::GradientSliderWidget(Kind kind, QWidget *parent)
    : QWidget(parent), m_kind(kind)
{
    setFixedHeight(20);
    setMinimumWidth(120);
    setCursor(Qt::PointingHandCursor);
}

void GradientSliderWidget::setBaseColor(const QColor &color)
{
    m_baseColor = color;
    if (m_kind == Kind::Alpha) {
        update();
    }
}

void GradientSliderWidget::setValue(int value)
{
    m_value = qBound(0, value, maximumValue());
    update();
}

int GradientSliderWidget::maximumValue() const
{
    return (m_kind == Kind::Hue) ? 359 : 255;
}

void GradientSliderWidget::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const QRectF area = rect().adjusted(0, 0, -1, -1);
    constexpr qreal RADIUS = 4.0;

    QPainterPath clipPath;
    clipPath.addRoundedRect(area, RADIUS, RADIUS);
    painter.setClipPath(clipPath);

    if (m_kind == Kind::Hue) {
        QLinearGradient gradient(area.topLeft(), area.topRight());
        for (int i = 0; i <= 360; i += 60) {
            gradient.setColorAt(i / 360.0, QColor::fromHsv(i % 360, 255, 255));
        }
        painter.fillRect(area, gradient);
    } else {
        // 市松模様(透明度が分かるように)
        constexpr int CHECKER_SIZE = 6;
        for (int y = 0; y < area.height(); y += CHECKER_SIZE) {
            for (int x = 0; x < area.width(); x += CHECKER_SIZE) {
                const bool dark = ((x / CHECKER_SIZE) + (y / CHECKER_SIZE)) % 2 == 0;
                painter.fillRect(QRectF(x, y, CHECKER_SIZE, CHECKER_SIZE),
                                  dark ? QColor(200, 200, 200) : QColor(255, 255, 255));
            }
        }

        QColor transparentBase = m_baseColor;
        transparentBase.setAlpha(0);
        QColor opaqueBase = m_baseColor;
        opaqueBase.setAlpha(255);

        QLinearGradient gradient(area.topLeft(), area.topRight());
        gradient.setColorAt(0.0, transparentBase);
        gradient.setColorAt(1.0, opaqueBase);
        painter.fillRect(area, gradient);
    }

    painter.setClipping(false);
    painter.setPen(QPen(palette().color(QPalette::Mid)));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(area, RADIUS, RADIUS);

    // つまみ
    const qreal fraction = static_cast<qreal>(m_value) / maximumValue();
    const qreal x = fraction * area.width();

    painter.setPen(QPen(Qt::white, 2));
    painter.setBrush(Qt::NoBrush);
    painter.drawLine(QPointF(x, 0), QPointF(x, area.height()));
    painter.setPen(QPen(Qt::black, 1));
    painter.drawLine(QPointF(x, 0), QPointF(x, area.height()));
}

void GradientSliderWidget::mousePressEvent(QMouseEvent *event)
{
    updateFromPos(event->pos());
}

void GradientSliderWidget::mouseMoveEvent(QMouseEvent *event)
{
    if ((event->buttons() & Qt::LeftButton) != 0) {
        updateFromPos(event->pos());
    }
}

void GradientSliderWidget::updateFromPos(const QPoint &pos)
{
    const qreal w = qMax(1, width() - 1);
    const qreal fraction = qBound(0.0, pos.x() / w, 1.0);
    m_value = qRound(fraction * maximumValue());

    update();
    emit valueChanged(m_value);
}

// ============================================================================
// ColorConverterGUI
// ============================================================================

ColorConverterGUI::ColorConverterGUI(ColorConverter *converter, QWidget *parent)
    : QWidget(parent), m_converter(converter)
{
    m_converter->setParent(this);

    auto *const mainLayout = new QHBoxLayout(this);
    mainLayout->addWidget(createPickerCard(), 0);
    mainLayout->addWidget(createValuesCard(), 1);

    connect(m_converter, &ColorConverter::currentColorChanged, this,
            &ColorConverterGUI::refreshFromColor);
    connect(m_converter, &ColorConverter::comparisonChanged, this,
            &ColorConverterGUI::refreshComparison);
    connect(m_converter, &ColorConverter::historyChanged, this,
            &ColorConverterGUI::rebuildHistoryRow);

    refreshFromColor(m_converter->currentColor());
    refreshComparison();
    rebuildHistoryRow();
}

QWidget *ColorConverterGUI::createPickerCard()
{
    auto *const card = new QWidget(this);
    auto *const layout = new QVBoxLayout(card);

    auto *const title = new QLabel(tr("カラーピッカー"), card);
    QFont titleFont = title->font();
    titleFont.setBold(true);
    title->setFont(titleFont);
    layout->addWidget(title);

    m_svPicker = new SvPickerWidget(card);
    layout->addWidget(m_svPicker);

    m_hueSlider = new GradientSliderWidget(GradientSliderWidget::Kind::Hue, card);
    layout->addWidget(m_hueSlider);

    m_alphaSlider = new GradientSliderWidget(GradientSliderWidget::Kind::Alpha, card);
    layout->addWidget(m_alphaSlider);

    auto *const previewRow = new QHBoxLayout();
    m_previewSwatch = createSwatch(48);
    previewRow->addWidget(m_previewSwatch);

    m_eyedropperButton = new QToolButton(card);
    m_eyedropperButton->setText(tr("スポイト"));
    m_eyedropperButton->setToolTip(tr("画面上の色を選択"));
    QIcon eyedropperIcon = QIcon::fromTheme("color-picker");
    if (eyedropperIcon.isNull()) {
        eyedropperIcon = QIcon::fromTheme("gtk-color-picker");
    }
    if (!eyedropperIcon.isNull()) {
        m_eyedropperButton->setIcon(eyedropperIcon);
    }
    previewRow->addWidget(m_eyedropperButton);
    previewRow->addStretch();
    layout->addLayout(previewRow);

    auto *const historyTitle = new QLabel(tr("履歴"), card);
    QFont historyFont = historyTitle->font();
    historyFont.setBold(true);
    historyTitle->setFont(historyFont);
    layout->addWidget(historyTitle);

    m_historyRow = new QWidget(card);
    m_historyRowLayout = new QHBoxLayout(m_historyRow);
    m_historyRowLayout->setContentsMargins(0, 0, 0, 0);
    m_historyRowLayout->setSpacing(4);
    layout->addWidget(m_historyRow);

    layout->addStretch();

    connect(m_svPicker, &SvPickerWidget::saturationValueChanged, this,
            &ColorConverterGUI::onSvChanged);
    connect(m_hueSlider, &GradientSliderWidget::valueChanged, this,
            &ColorConverterGUI::onHueChanged);
    connect(m_alphaSlider, &GradientSliderWidget::valueChanged, this,
            &ColorConverterGUI::onAlphaChanged);
    connect(m_eyedropperButton, &QToolButton::clicked, this,
            &ColorConverterGUI::onEyedropperClicked);

    return card;
}

QWidget *ColorConverterGUI::createValuesCard()
{
    auto *const card = new QWidget(this);
    auto *const outer = new QVBoxLayout(card);

    auto *const title = new QLabel(tr("値"), card);
    QFont titleFont = title->font();
    titleFont.setBold(true);
    title->setFont(titleFont);
    outer->addWidget(title);

    auto *const grid = new QGridLayout();
    grid->setColumnStretch(1, 1);
    createValueRow(tr("HEX"), &m_hexInput, grid, 0, card);
    createValueRow(tr("RGB"), &m_rgbInput, grid, 1, card);
    createValueRow(tr("HSL"), &m_hslInput, grid, 2, card);
    createValueRow(tr("HSV"), &m_hsvInput, grid, 3, card);
    createValueRow(tr("CMYK"), &m_cmykInput, grid, 4, card);
    outer->addLayout(grid);

    connect(m_hexInput, &QLineEdit::editingFinished, this, &ColorConverterGUI::onHexEdited);
    connect(m_rgbInput, &QLineEdit::editingFinished, this, &ColorConverterGUI::onRgbEdited);
    connect(m_hslInput, &QLineEdit::editingFinished, this, &ColorConverterGUI::onHslEdited);
    connect(m_hsvInput, &QLineEdit::editingFinished, this, &ColorConverterGUI::onHsvEdited);
    connect(m_cmykInput, &QLineEdit::editingFinished, this, &ColorConverterGUI::onCmykEdited);

    auto *const comparisonTitle = new QLabel(tr("比較"), card);
    QFont comparisonFont = comparisonTitle->font();
    comparisonFont.setBold(true);
    comparisonTitle->setFont(comparisonFont);
    outer->addWidget(comparisonTitle);

    outer->addWidget(createComparisonPanel());
    outer->addStretch();

    return card;
}

void ColorConverterGUI::createValueRow(const QString &labelText, QLineEdit **inputOut,
                                        QGridLayout *grid, int row, QWidget *rowParent)
{
    auto *const label = new QLabel(labelText, rowParent);
    auto *const edit = new QLineEdit(rowParent);
    auto *const copyButton = new QToolButton(rowParent);
    copyButton->setText(tr("コピー"));
    copyButton->setToolTip(tr("クリップボードにコピー"));
    QIcon copyIcon = QIcon::fromTheme("edit-copy");
    if (!copyIcon.isNull()) {
        copyButton->setIcon(copyIcon);
    }

    grid->addWidget(label, row, 0);
    grid->addWidget(edit, row, 1);
    grid->addWidget(copyButton, row, 2);

    connect(copyButton, &QToolButton::clicked, this, [this, edit]() { onCopyRequested(edit); });

    *inputOut = edit;
}

QWidget *ColorConverterGUI::createComparisonPanel()
{
    auto *const panel = new QWidget(this);
    auto *const outer = new QVBoxLayout(panel);

    auto *const row = new QHBoxLayout();

    auto *const fromColumn = new QVBoxLayout();
    m_fromSwatch = createSwatch(40);
    fromColumn->addWidget(m_fromSwatch, 0, Qt::AlignHCenter);
    fromColumn->addWidget(new QLabel(tr("From"), panel), 0, Qt::AlignHCenter);
    row->addLayout(fromColumn);

    auto *const midColumn = new QVBoxLayout();
    m_deltaELabel = new QLabel(panel);
    m_deltaELabel->setAlignment(Qt::AlignCenter);
    midColumn->addWidget(m_deltaELabel);

    auto *const contrastRow = new QHBoxLayout();
    m_contrastLabel = new QLabel(panel);
    contrastRow->addWidget(m_contrastLabel);
    m_contrastBadge = new QLabel(panel);
    m_contrastBadge->setAlignment(Qt::AlignCenter);
    m_contrastBadge->setStyleSheet(
        "QLabel { border-radius: 4px; padding: 1px 6px; background-color: palette(mid); }");
    contrastRow->addWidget(m_contrastBadge);
    midColumn->addLayout(contrastRow);
    row->addLayout(midColumn, 1);

    auto *const toColumn = new QVBoxLayout();
    m_toSwatch = createSwatch(40);
    toColumn->addWidget(m_toSwatch, 0, Qt::AlignHCenter);
    toColumn->addWidget(new QLabel(tr("To"), panel), 0, Qt::AlignHCenter);
    row->addLayout(toColumn);

    outer->addLayout(row);

    auto *const actionsRow = new QHBoxLayout();
    m_lockFromButton = new QPushButton(tr("Fromに固定"), panel);
    m_swapButton = new QPushButton(tr("入れ替え"), panel);
    actionsRow->addWidget(m_lockFromButton);
    actionsRow->addWidget(m_swapButton);
    outer->addLayout(actionsRow);

    connect(m_lockFromButton, &QPushButton::clicked, m_converter,
            &ColorConverter::lockCurrentAsFrom);
    connect(m_swapButton, &QPushButton::clicked, m_converter, &ColorConverter::swapFromTo);

    return panel;
}

QWidget *ColorConverterGUI::createSwatch(int size)
{
    auto *const swatch = new QWidget();
    swatch->setFixedSize(size, size);
    return swatch;
}

void ColorConverterGUI::setSwatchColor(QWidget *swatch, const QColor &color)
{
    swatch->setStyleSheet(QString("background-color: rgba(%1, %2, %3, %4); "
                                   "border: 1px solid palette(mid); border-radius: 6px;")
                               .arg(color.red())
                               .arg(color.green())
                               .arg(color.blue())
                               .arg(color.alpha()));
}

void ColorConverterGUI::rebuildHistoryRow()
{
    QLayoutItem *item = nullptr;
    while ((item = m_historyRowLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    for (const QColor &color : m_converter->history()) {
        auto *const button = new QToolButton(m_historyRow);
        button->setFixedSize(24, 24);
        button->setToolTip(ColorConverter::toHex(color));
        button->setStyleSheet(QString("QToolButton { background-color: %1; "
                                       "border: 1px solid palette(mid); border-radius: 4px; }")
                                   .arg(color.name()));
        connect(button, &QToolButton::clicked, this,
                [this, color]() { onHistorySwatchClicked(color); });
        m_historyRowLayout->addWidget(button);
    }
    m_historyRowLayout->addStretch();
}

void ColorConverterGUI::refreshFromColor(const QColor &color)
{
    // ピッカー系ウィジェットをプログラム的に更新する(これらはsetterで値変更してもシグナルを
    // 発火しないため、フィードバックループにはならない)
    qreal h = 0.0;
    qreal s = 0.0;
    qreal v = 0.0;
    qreal a = 0.0;
    color.getHsvF(&h, &s, &v, &a);
    if (h < 0.0) {
        h = 0.0;
    }

    m_hueSlider->setValue(qRound(h * 359.0));
    m_svPicker->setHue(h);
    m_svPicker->setSaturationValue(s, v);
    m_alphaSlider->setBaseColor(color);
    m_alphaSlider->setValue(color.alpha());

    setSwatchColor(m_previewSwatch, color);

    m_hexInput->setText(ColorConverter::toHex(color, color.alpha() < 255));
    m_rgbInput->setText(ColorConverter::toRgbString(color));
    m_hslInput->setText(ColorConverter::toHslString(color));
    m_hsvInput->setText(ColorConverter::toHsvString(color));
    m_cmykInput->setText(ColorConverter::toCmykString(color));
}

void ColorConverterGUI::refreshComparison()
{
    setSwatchColor(m_fromSwatch, m_converter->fromColor());
    setSwatchColor(m_toSwatch, m_converter->toColor());

    const double deltaE = m_converter->comparisonDeltaE();
    m_deltaELabel->setText(tr("ΔE  %1").arg(QString::number(deltaE, 'f', 1)));

    const double ratio = m_converter->comparisonContrastRatio();
    m_contrastLabel->setText(tr("コントラスト %1:1").arg(QString::number(ratio, 'f', 2)));

    const ColorConverter::ContrastLevel level = ColorConverter::contrastLevel(ratio);
    m_contrastBadge->setText(ColorConverter::contrastLevelLabel(level));
    const bool passes = level != ColorConverter::ContrastLevel::Fail;
    m_contrastBadge->setStyleSheet(
        QString("QLabel { border-radius: 4px; padding: 1px 6px; color: white; "
                "background-color: %1; }")
            .arg(passes ? "#1a7f37" : "#d93c15"));
}

void ColorConverterGUI::onSvChanged(qreal saturation, qreal value)
{
    const qreal hue = m_hueSlider->value() / 359.0;
    QColor color = QColor::fromHsvF(hue, saturation, value);
    color.setAlpha(m_alphaSlider->value());
    m_converter->setCurrentColor(color);
}

void ColorConverterGUI::onHueChanged(int hue)
{
    QColor color = QColor::fromHsvF(hue / 359.0, m_svPicker->saturation(), m_svPicker->value());
    color.setAlpha(m_alphaSlider->value());
    m_converter->setCurrentColor(color);
}

void ColorConverterGUI::onAlphaChanged(int alpha)
{
    QColor color = m_converter->currentColor();
    color.setAlpha(alpha);
    m_converter->setCurrentColor(color);
}

void ColorConverterGUI::onHexEdited()
{
    bool ok = false;
    const QColor color = ColorConverter::fromHex(m_hexInput->text(), &ok);
    if (ok) {
        m_converter->setCurrentColor(color);
    } else {
        refreshFromColor(m_converter->currentColor());
    }
}

void ColorConverterGUI::onRgbEdited()
{
    bool ok = false;
    const QColor color = ColorConverter::fromRgbString(m_rgbInput->text(), &ok);
    if (ok) {
        m_converter->setCurrentColor(color);
    } else {
        refreshFromColor(m_converter->currentColor());
    }
}

void ColorConverterGUI::onHslEdited()
{
    bool ok = false;
    const QColor color = ColorConverter::fromHslString(m_hslInput->text(), &ok);
    if (ok) {
        m_converter->setCurrentColor(color);
    } else {
        refreshFromColor(m_converter->currentColor());
    }
}

void ColorConverterGUI::onHsvEdited()
{
    bool ok = false;
    const QColor color = ColorConverter::fromHsvString(m_hsvInput->text(), &ok);
    if (ok) {
        m_converter->setCurrentColor(color);
    } else {
        refreshFromColor(m_converter->currentColor());
    }
}

void ColorConverterGUI::onCmykEdited()
{
    bool ok = false;
    const QColor color = ColorConverter::fromCmykString(m_cmykInput->text(), &ok);
    if (ok) {
        m_converter->setCurrentColor(color);
    } else {
        refreshFromColor(m_converter->currentColor());
    }
}

void ColorConverterGUI::onCopyRequested(const QLineEdit *source)
{
    QGuiApplication::clipboard()->setText(source->text());
}

void ColorConverterGUI::onEyedropperClicked()
{
    // NOTE: QScreen::grabWindow による簡易スポイト実装。
    //       マウスをグラブし、次のクリック位置の画面上の色を1回だけ取得する。
    //       (グラブ中はこのウィジェット自身が全てのマウスイベントを受け取る)
    if (m_pickingFromScreen) {
        return;
    }
    m_pickingFromScreen = true;
    grabMouse(Qt::CrossCursor);
}

void ColorConverterGUI::mousePressEvent(QMouseEvent *event)
{
    if (!m_pickingFromScreen) {
        QWidget::mousePressEvent(event);
        return;
    }

    const QPoint globalPos = event->globalPosition().toPoint();
    if (QScreen *screen = QGuiApplication::screenAt(globalPos)) {
        const QPixmap pixmap = screen->grabWindow(0, globalPos.x(), globalPos.y(), 1, 1);
        if (!pixmap.isNull()) {
            const QColor picked = pixmap.toImage().pixelColor(0, 0);
            m_converter->setCurrentColor(picked);
        }
    }

    releaseMouse();
    m_pickingFromScreen = false;
    event->accept();
}

void ColorConverterGUI::onHistorySwatchClicked(const QColor &color)
{
    m_converter->setCurrentColor(color);
}
