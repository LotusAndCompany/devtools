#include "design_system.h"

#include <QAbstractButton>
#include <QAbstractItemView>
#include <QAbstractScrollArea>
#include <QApplication>
#include <QBoxLayout>
#include <QFontDatabase>
#include <QFormLayout>
#include <QFrame>
#include <QGraphicsOpacityEffect>
#include <QGridLayout>
#include <QIcon>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QSizePolicy>
#include <QSplitter>
#include <QStyleOption>
#include <QTextBrowser>
#include <QToolBar>
#include <QToolButton>
#include <QWidget>

#include <oclero/qlementine/Common.hpp>
#include <oclero/qlementine/style/QlementineStyle.hpp>
#include <oclero/qlementine/widgets/Label.hpp>

namespace DevTools::Ui {

namespace {
void applyErrorColor(QLabel *label, const oclero::qlementine::QlementineStyle *style)
{
    const QColor errorColor = style->statusColor(oclero::qlementine::Status::Error,
                                                 oclero::qlementine::MouseState::Normal);
    const QColor disabledErrorColor = style->statusColor(oclero::qlementine::Status::Error,
                                                         oclero::qlementine::MouseState::Disabled);

    QPalette palette = label->palette();
    palette.setColor(QPalette::Active, QPalette::WindowText, errorColor);
    palette.setColor(QPalette::Inactive, QPalette::WindowText, errorColor);
    palette.setColor(QPalette::Normal, QPalette::WindowText, errorColor);
    palette.setColor(QPalette::Disabled, QPalette::WindowText, disabledErrorColor);
    label->setPalette(palette);
}

int textControlBorderWidth(const QWidget *control)
{
    if (auto *const qlementineStyle =
            qobject_cast<oclero::qlementine::QlementineStyle *>(control->style())) {
        return qlementineStyle->theme().borderWidth;
    }
    return 1;
}

class QlementineTextBrowser final : public QTextBrowser
{
public:
    using QTextBrowser::QTextBrowser;
    using QTextBrowser::setViewportMargins;

protected:
    void paintEvent(QPaintEvent *event) override;
};

void setTextControlViewportMargins(QAbstractScrollArea *control, int borderWidth)
{
    if (auto *const textBrowser = dynamic_cast<QlementineTextBrowser *>(control)) {
        textBrowser->setViewportMargins(borderWidth, borderWidth, borderWidth, borderWidth);
    }
}

void drawTextControlBorder(QAbstractScrollArea *control)
{
    auto *const qlementineStyle =
        qobject_cast<oclero::qlementine::QlementineStyle *>(control->style());
    if (qlementineStyle == nullptr) {
        return;
    }

    const int borderWidth = qlementineStyle->theme().borderWidth;
    QWidget *const viewport = control->viewport();
    QStyleOptionFrame option;
    option.initFrom(control);
    option.rect = viewport->rect();
    option.lineWidth = borderWidth;

    QPainter painter(viewport);
    const qreal radius = qlementineStyle->theme().borderRadius;
    const qreal innerRadius = qMax<qreal>(0.0, radius - borderWidth);
    QPainterPath borderPath;
    borderPath.addRoundedRect(QRectF(viewport->rect()), radius, radius);
    QPainterPath innerPath;
    innerPath.addRoundedRect(
        QRectF(viewport->rect().adjusted(borderWidth, borderWidth, -borderWidth, -borderWidth)),
        innerRadius, innerRadius);
    painter.setClipPath(borderPath.subtracted(innerPath));
    qlementineStyle->drawPrimitive(QStyle::PE_PanelLineEdit, &option, &painter, control);
}

void QlementineTextBrowser::paintEvent(QPaintEvent *event)
{
    QTextBrowser::paintEvent(event);
    drawTextControlBorder(this);
}

} // namespace

QMargins pageMargins()
{
    return {Metrics::PAGE_MARGIN, Metrics::PAGE_MARGIN, Metrics::PAGE_MARGIN, Metrics::PAGE_MARGIN};
}

QMargins panelMargins()
{
    return {Metrics::PANEL_MARGIN, Metrics::PANEL_MARGIN, Metrics::PANEL_MARGIN,
            Metrics::PANEL_MARGIN};
}

QMargins toolbarMargins()
{
    return {Metrics::TOOLBAR_MARGIN, 0, Metrics::TOOLBAR_MARGIN, 0};
}

QSize mainWindowSize()
{
    return {1280, 720};
}

QSize dialogSize()
{
    return {640, 480};
}

void applyPageLayout(QLayout *layout)
{
    layout->setContentsMargins(pageMargins());
    layout->setSpacing(Metrics::PAGE_SPACING);
}

void applyPanelLayout(QLayout *layout)
{
    layout->setContentsMargins(panelMargins());
    layout->setSpacing(Metrics::CONTROL_SPACING);
}

void applyToolbarLayout(QLayout *layout)
{
    layout->setContentsMargins(toolbarMargins());
    layout->setSpacing(Metrics::CONTROL_SPACING);
}

void applyInlineLayout(QLayout *layout)
{
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(Metrics::COMPACT_SPACING);
}

void applyFullBleedLayout(QLayout *layout)
{
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
}

void applySplitLayout(QLayout *layout)
{
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(Metrics::PAGE_SPACING);
}

void configureFormLayout(QFormLayout *layout)
{
    layout->setContentsMargins(panelMargins());
    layout->setHorizontalSpacing(Metrics::CONTROL_SPACING);
    layout->setVerticalSpacing(Metrics::CONTROL_SPACING);
    layout->setSizeConstraint(QLayout::SetMinimumSize);
}

void configureInlineFormLayout(QFormLayout *layout)
{
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setHorizontalSpacing(Metrics::CONTROL_SPACING);
    layout->setVerticalSpacing(Metrics::CONTROL_SPACING);
    layout->setSizeConstraint(QLayout::SetMinimumSize);
}

void configureActionBar(QBoxLayout *layout, ActionBarAlignment alignment)
{
    applyToolbarLayout(layout);
    if (alignment == ActionBarAlignment::Trailing) {
        layout->insertStretch(0, 1);
    }
}

void configureEqualLayout(QBoxLayout *layout)
{
    layout->setStretch(0, Metrics::EQUAL_SPLIT_STRETCH);
    layout->setStretch(1, Metrics::EQUAL_SPLIT_STRETCH);
}

void configureMainSideLayout(QBoxLayout *layout)
{
    layout->setStretch(0, Metrics::MAIN_PANEL_STRETCH);
    layout->setStretch(1, Metrics::SIDE_PANEL_STRETCH);
}

void configureSideMainLayout(QBoxLayout *layout)
{
    layout->setStretch(0, Metrics::SIDE_PANEL_STRETCH);
    layout->setStretch(1, Metrics::MAIN_PANEL_STRETCH);
}

void configureEqualSplitter(QSplitter *splitter)
{
    splitter->setHandleWidth(Metrics::SPLITTER_HANDLE_WIDTH);
    splitter->setStretchFactor(0, Metrics::EQUAL_SPLIT_STRETCH);
    splitter->setStretchFactor(1, Metrics::EQUAL_SPLIT_STRETCH);
}

void configureRequestOptionsResultSplitter(QSplitter *splitter)
{
    splitter->setHandleWidth(Metrics::SPLITTER_HANDLE_WIDTH);
    splitter->setStretchFactor(0, Metrics::SIDE_PANEL_STRETCH);
    splitter->setStretchFactor(1, Metrics::SIDE_PANEL_STRETCH);
    splitter->setStretchFactor(2, Metrics::MAIN_PANEL_STRETCH);
}

void configureCaptionValueGrid(QGridLayout *layout)
{
    layout->setColumnStretch(0, Metrics::CAPTION_COLUMN_STRETCH);
    layout->setColumnStretch(1, Metrics::VALUE_COLUMN_STRETCH);
}

void configureTextToolbar(QToolBar *toolbar)
{
    toolbar->setToolButtonStyle(Qt::ToolButtonTextOnly);
}

void addBottomRightOverlay(QGridLayout *layout, QWidget *overlay)
{
    layout->addWidget(overlay, 0, 0, Qt::AlignRight | Qt::AlignBottom);
}

void configureMainWindow(QWidget *window)
{
    window->resize(mainWindowSize());
}

void configureDialog(QWidget *dialog)
{
    dialog->resize(dialogSize());
}

QLabel *createPaneHeading(const QString &text, QWidget *parent)
{
    auto *const label =
        new oclero::qlementine::Label(text, oclero::qlementine::TextRole::H5, parent);
    configurePaneHeading(label);
    return label;
}

void configurePaneHeading(QLabel *label)
{
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    if (auto *const qlementineLabel = qobject_cast<oclero::qlementine::Label *>(label)) {
        qlementineLabel->setRole(oclero::qlementine::TextRole::H5);
        return;
    }

    auto *const qlementineStyle =
        qobject_cast<oclero::qlementine::QlementineStyle *>(label->style());
    if (qlementineStyle != nullptr) {
        label->setFont(qlementineStyle->fontForTextRole(oclero::qlementine::TextRole::H5));
        label->setPalette(qlementineStyle->paletteForTextRole(oclero::qlementine::TextRole::H5));
    }
}

void configureHeroLabel(QLabel *label)
{
    QFont font = label->font();
    font.setPointSize(Metrics::HERO_POINT_SIZE);
    label->setFont(font);
    label->setAlignment(Qt::AlignCenter);
}

void configureDisplayTitle(QLabel *label)
{
    QFont font = label->font();
    font.setPointSize(Metrics::TITLE_POINT_SIZE);
    font.setBold(true);
    label->setFont(font);
}

void configureLogoLabel(QLabel *label)
{
    label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    label->setFixedSize(Metrics::LOGO_SIZE, Metrics::LOGO_SIZE);
    label->setScaledContents(true);
}

void configureCenteredLabel(QLabel *label)
{
    label->setAlignment(Qt::AlignCenter);
}

void configureCaptionLabel(QLabel *label)
{
    label->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
}

void configureSidebarItem(QPushButton *button)
{
    button->setFlat(true);
    button->setIconSize(QSize(Metrics::ICON_SIZE, Metrics::ICON_SIZE));
}

void configurePrimaryButton(QAbstractButton *button)
{
    button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void configureCompactButton(QAbstractButton *button)
{
    button->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
}

void configureDivider(QFrame *divider)
{
    divider->setFrameShape(QFrame::HLine);
    divider->setFrameShadow(QFrame::Sunken);
    divider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

QLineEdit *createLineEdit(QWidget *parent)
{
    return new QLineEdit(parent);
}

QPlainTextEdit *createPlainTextEdit(QWidget *parent)
{
    return new QPlainTextEdit(parent);
}

QTextBrowser *createTextBrowser(QWidget *parent)
{
    return new QlementineTextBrowser(parent);
}

void configureDocumentPreview(QTextBrowser *preview)
{
    configureTextControl(preview);
    preview->setFont(QFontDatabase::systemFont(QFontDatabase::GeneralFont));
}

void configureFormField(QWidget *field)
{
    if (auto *const lineEdit = qobject_cast<QLineEdit *>(field)) {
        configureTextControl(lineEdit);
    }
    field->setMinimumWidth(Metrics::FIELD_MIN_WIDTH);
    field->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void configureMultilineField(QPlainTextEdit *field)
{
    configureTextControl(field);
    field->setMinimumHeight(Metrics::MULTILINE_EDITOR_MIN_HEIGHT);
    field->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void configurePreviewEditor(QPlainTextEdit *editor)
{
    configureCodeEditor(editor);
    editor->setMaximumHeight(Metrics::STATUS_VIEW_HEIGHT);
}

void configureStatusView(QPlainTextEdit *view)
{
    configureTextControl(view);
    view->setReadOnly(true);
    view->setMaximumHeight(Metrics::STATUS_VIEW_HEIGHT);
    view->setTextInteractionFlags(Qt::TextSelectableByKeyboard | Qt::TextSelectableByMouse);
}

void configurePreviewSurface(QLabel *surface)
{
    surface->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    surface->setMinimumSize(Metrics::PREVIEW_MIN_SIZE, Metrics::PREVIEW_MIN_SIZE);
    surface->setFrameShape(QFrame::Box);
    surface->setAlignment(Qt::AlignCenter);
}

void configureColorSample(QFrame *sample)
{
    sample->setFixedSize(Metrics::ICON_BUTTON_SIZE, Metrics::ICON_BUTTON_SIZE);
    sample->setFrameStyle(QFrame::Panel | QFrame::Raised);
}

void configureExpandingSurface(QWidget *surface)
{
    surface->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void configurePaneSurface(QWidget *surface)
{
    auto *const qlementineStyle =
        qobject_cast<oclero::qlementine::QlementineStyle *>(surface->style());
    if (qlementineStyle == nullptr) {
        return;
    }

    const QColor background =
        qlementineStyle->groupBoxBackgroundColor(oclero::qlementine::MouseState::Normal);
    QPalette palette = surface->palette();
    for (const auto group :
         {QPalette::Active, QPalette::Inactive, QPalette::Disabled, QPalette::Normal}) {
        palette.setColor(group, QPalette::Window, background);
        palette.setColor(group, QPalette::Base, background);
    }
    surface->setPalette(palette);
    surface->setAutoFillBackground(true);

    if (auto *const scrollArea = qobject_cast<QAbstractScrollArea *>(surface)) {
        scrollArea->viewport()->setPalette(palette);
        scrollArea->viewport()->setAutoFillBackground(true);
    }
}

void configureImageSurface(QLabel *surface)
{
    configureExpandingSurface(surface);
    configureCenteredLabel(surface);
}

void configureScrollView(QScrollArea *view)
{
    configureExpandingSurface(view);
    view->setFrameShape(QFrame::NoFrame);
    view->setWidgetResizable(true);
    view->setAlignment(Qt::AlignCenter);
}

void configureAlternatingList(QAbstractItemView *view)
{
    view->setAlternatingRowColors(true);
}

void configureSidebar(QWidget *sidebar)
{
    sidebar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    sidebar->setMinimumWidth(Metrics::SIDEMENU_WIDTH);
}

void configureToolbarSpacer(QWidget *spacer)
{
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
}

void configureSection(QFrame *section)
{
    section->setFrameShape(QFrame::StyledPanel);
}

void configureToolPanel(QScrollArea *panel)
{
    panel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    panel->setMinimumWidth(Metrics::TOOL_PANEL_WIDTH);
    panel->setWidgetResizable(true);
    panel->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    panel->setFrameShape(QFrame::NoFrame);
}

void configureZoomButton(QToolButton *button, const QString &iconName, const QString &toolTip)
{
    configureIconButton(button, iconName, toolTip);
    button->setAutoFillBackground(true);

    const QIcon icon = QIcon::fromTheme(iconName);
    if (icon.isNull()) {
        button->setText(toolTip);
    }

    auto *const opacityEffect = new QGraphicsOpacityEffect(button);
    opacityEffect->setOpacity(Metrics::ZOOM_BUTTON_OPACITY);
    button->setGraphicsEffect(opacityEffect);
}

QLabel *createErrorLabel(QWidget *parent)
{
    auto *const label = new QLabel(parent);
    configureErrorLabel(label);
    label->setVisible(false);
    return label;
}

QSize previewContentSize(const QSize &surfaceSize)
{
    return (surfaceSize - QSize(2 * Metrics::PREVIEW_INSET, 2 * Metrics::PREVIEW_INSET))
        .expandedTo(QSize(1, 1));
}

void configureIconButton(QAbstractButton *button, const QString &iconName, const QString &toolTip)
{
    button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    button->setFixedSize(Metrics::ICON_BUTTON_SIZE, Metrics::ICON_BUTTON_SIZE);
    button->setIcon(QIcon::fromTheme(iconName));
    button->setIconSize(QSize(Metrics::ICON_SIZE, Metrics::ICON_SIZE));
    if (!toolTip.isEmpty()) {
        button->setToolTip(toolTip);
    }
}

void configureWindowControlButton(QPushButton *button, const QString &iconName,
                                  const QString &toolTip)
{
    configureIconButton(button, iconName, toolTip);
    button->setFlat(true);
}

void configureErrorLabel(QLabel *label)
{
    label->setProperty("devtoolsStatus", QStringLiteral("error"));
    label->setForegroundRole(QPalette::WindowText);

    auto *const qlementineStyle =
        qobject_cast<oclero::qlementine::QlementineStyle *>(QApplication::style());
    if (qlementineStyle != nullptr) {
        applyErrorColor(label, qlementineStyle);
    }
}

void configureTextControl(QLineEdit *control)
{
    control->setFrame(true);
}

void configureTextControl(QAbstractScrollArea *control)
{
    const int borderWidth = textControlBorderWidth(control);
    control->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
    control->setLineWidth(borderWidth);
    setTextControlViewportMargins(control, borderWidth);
    control->viewport()->setAutoFillBackground(false);
    control->viewport()->setAttribute(Qt::WA_OpaquePaintEvent, false);
}

void refreshStatusColors()
{
    auto *const qlementineStyle =
        qobject_cast<oclero::qlementine::QlementineStyle *>(QApplication::style());
    if (qlementineStyle == nullptr) {
        return;
    }

    // ponytail: scan the small widget tree; add a status-label registry only if this is measurable.
    for (QWidget *const widget : QApplication::allWidgets()) {
        auto *const label = qobject_cast<QLabel *>(widget);
        if (label == nullptr || label->property("devtoolsStatus").toString() != "error") {
            continue;
        }

        applyErrorColor(label, qlementineStyle);
    }
}

QFont codeFont()
{
    return QFontDatabase::systemFont(QFontDatabase::FixedFont);
}

void configureCodeEditor(QPlainTextEdit *editor)
{
    configureTextControl(editor);
    editor->setFont(codeFont());
    editor->setTabStopDistance(Metrics::CODE_TAB_STOP);
}

} // namespace DevTools::Ui
