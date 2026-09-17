#include "design_system.h"

#include <QAbstractButton>
#include <QAbstractItemView>
#include <QAbstractScrollArea>
#include <QApplication>
#include <QBoxLayout>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFontDatabase>
#include <QFormLayout>
#include <QFrame>
#include <QGraphicsOpacityEffect>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QIcon>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QListView>
#include <QListWidgetItem>
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
#include <QStyledItemDelegate>
#include <QTableView>
#include <QTextBrowser>
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

class RoundedListItemDelegate final : public QStyledItemDelegate
{
public:
    RoundedListItemDelegate(const QColor &normalRowColor, const QColor &alternateRowColor,
                            QObject *parent = nullptr)
        : QStyledItemDelegate(parent)
        , normal_row_color(normalRowColor)
        , alternate_row_color(alternateRowColor)
    {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override
    {
        const QRect itemRect = option.rect.adjusted(
            Metrics::LIST_ROW_HORIZONTAL_INSET, Metrics::LIST_ROW_VERTICAL_INSET,
            -Metrics::LIST_ROW_HORIZONTAL_INSET, -Metrics::LIST_ROW_VERTICAL_INSET);
        const qreal radius =
            qMin<qreal>(Metrics::CORNER_RADIUS, qMin(itemRect.width(), itemRect.height()) / 2.0);

        QPainterPath roundedPath;
        roundedPath.addRoundedRect(QRectF(itemRect), radius, radius);
        const QColor &rowColor = option.features.testFlag(QStyleOptionViewItem::Alternate)
                                     ? alternate_row_color
                                     : normal_row_color;

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(Qt::NoPen);
        painter->setBrush(rowColor);
        painter->drawPath(roundedPath);
        painter->setClipPath(roundedPath);

        QStyleOptionViewItem roundedOption(option);
        roundedOption.rect = itemRect;
        QStyledItemDelegate::paint(painter, roundedOption, index);
        painter->restore();
    }

private:
    QColor normal_row_color;
    QColor alternate_row_color;
};

class RoundedListStyle final : public oclero::qlementine::QlementineStyle
{
public:
    explicit RoundedListStyle(const oclero::qlementine::QlementineStyle &source,
                              QObject *parent = nullptr)
        : QlementineStyle(parent), source_style(&source)
    {
        syncTheme();
        QObject::connect(&source, &QlementineStyle::themeChanged, this, [this]() { syncTheme(); });
    }

    void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter,
                       const QWidget *widget = nullptr) const override
    {
        if (element == PE_PanelItemViewRow) {
            return;
        }

        QlementineStyle::drawPrimitive(element, option, painter, widget);
    }

private:
    void syncTheme()
    {
        setTheme(source_style->theme());
        setAutoIconColor(source_style->autoIconColor());
    }

    const oclero::qlementine::QlementineStyle *source_style;
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

void applyContentLayout(QLayout *layout)
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

void configureEqualSplitter(QSplitter *splitter)
{
    splitter->setHandleWidth(Metrics::SPLITTER_HANDLE_WIDTH);
    splitter->setStretchFactor(0, Metrics::EQUAL_SPLIT_STRETCH);
    splitter->setStretchFactor(1, Metrics::EQUAL_SPLIT_STRETCH);
}

void configureMainSideSplitter(QSplitter *splitter)
{
    splitter->setHandleWidth(Metrics::SPLITTER_HANDLE_WIDTH);
    splitter->setStretchFactor(0, Metrics::MAIN_PANEL_STRETCH);
    splitter->setStretchFactor(1, Metrics::SIDE_PANEL_STRETCH);
}

void configureSideMainSplitter(QSplitter *splitter)
{
    splitter->setHandleWidth(Metrics::SPLITTER_HANDLE_WIDTH);
    splitter->setStretchFactor(0, Metrics::SIDE_PANEL_STRETCH);
    splitter->setStretchFactor(1, Metrics::MAIN_PANEL_STRETCH);
}

void configureCaptionValueGrid(QGridLayout *layout)
{
    layout->setColumnStretch(0, Metrics::CAPTION_COLUMN_STRETCH);
    layout->setColumnStretch(1, Metrics::VALUE_COLUMN_STRETCH);
}

void addBottomRightOverlay(QGridLayout *layout, QWidget *overlay)
{
    layout->addWidget(overlay, 0, 0, Qt::AlignRight | Qt::AlignBottom);
}

void configureDialog(QWidget *dialog)
{
    dialog->resize(dialogSize());
}

void configureDialogButtonBox(QDialogButtonBox *buttonBox)
{
    buttonBox->setOrientation(Qt::Horizontal);
    buttonBox->setCenterButtons(false);
    buttonBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

QGroupBox *createPane(const QString &title, QWidget *parent)
{
    auto *const pane = new QGroupBox(title, parent);
    configurePane(pane);
    return pane;
}

void configurePane(QGroupBox *pane)
{
    pane->setFlat(false);
    pane->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void configureCompactPane(QGroupBox *pane)
{
    pane->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
}

void configureToolbarPane(QGroupBox *pane)
{
    configureCompactPane(pane);
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

void configureCenteredLabel(QLabel *label)
{
    label->setAlignment(Qt::AlignCenter);
}

void configureHeroLabel(QLabel *label)
{
    auto heroFont = label->font();
    heroFont.setPointSize(Metrics::HERO_POINT_SIZE);
    label->setFont(heroFont);
    configureCenteredLabel(label);
}

void configureSidebarItem(QPushButton *button)
{
    button->setFlat(true);
    button->setIconSize(QSize(Metrics::ICON_SIZE, Metrics::ICON_SIZE));
    button->setStyleSheet(QStringLiteral("QPushButton { text-align: left; }"));
}

void configurePrimaryButton(QAbstractButton *button)
{
    button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void configureCompactButton(QAbstractButton *button)
{
    button->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
}

void configureTitleLabel(QLabel *label)
{
    auto titleFont = label->font();
    titleFont.setPointSize(Metrics::TITLE_POINT_SIZE);
    titleFont.setBold(true);
    label->setFont(titleFont);
}

void configureDivider(QFrame *divider)
{
    divider->setFrameShape(QFrame::HLine);
    divider->setFrameShadow(QFrame::Sunken);
    divider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

QTextBrowser *createTextBrowser(QWidget *parent)
{
    return new QlementineTextBrowser(parent);
}

void configureFormField(QWidget *field)
{
    field->setFont(standardFont());
    field->setMinimumWidth(Metrics::FIELD_MIN_WIDTH);
    field->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void configureLineEdit(QLineEdit *field)
{
    configureFormField(field);
}

void configureComboBox(QComboBox *field)
{
    configureFormField(field);
}

void configureCodeLineEdit(QLineEdit *field)
{
    configureLineEdit(field);
}

void configureItemView(QAbstractItemView *view)
{
    view->setFont(standardFont());
    view->setFrameStyle(QFrame::NoFrame);
    view->setLineWidth(0);
    view->setMidLineWidth(0);
    configurePaneSurface(view);
    view->setAlternatingRowColors(true);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->setSelectionMode(QAbstractItemView::SingleSelection);
    view->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    view->setTextElideMode(Qt::ElideRight);
    view->setIconSize(QSize(Metrics::ICON_SIZE, Metrics::ICON_SIZE));

    if (auto *const listView = qobject_cast<QListView *>(view)) {
        const int borderWidth = textControlBorderWidth(listView);
        int borderRadius = Metrics::CORNER_RADIUS;
        QColor borderColor = listView->palette().color(QPalette::Mid);
        QColor normalRowColor = listView->palette().color(QPalette::Base);
        QColor alternateRowColor = listView->palette().color(QPalette::AlternateBase);
        if (auto *const qlementineStyle =
                qobject_cast<oclero::qlementine::QlementineStyle *>(listView->style())) {
            borderRadius = static_cast<int>(qlementineStyle->theme().borderRadius);
            borderColor = qlementineStyle->theme().borderColor;
            normalRowColor = qlementineStyle->listItemRowBackgroundColor(
                oclero::qlementine::MouseState::Normal,
                oclero::qlementine::AlternateState::NotAlternate);
            alternateRowColor = qlementineStyle->listItemRowBackgroundColor(
                oclero::qlementine::MouseState::Normal,
                oclero::qlementine::AlternateState::Alternate);
            listView->setStyle(new RoundedListStyle(*qlementineStyle, listView));
        }

        listView->setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
        listView->setLineWidth(borderWidth);
        listView->setStyleSheet(QStringLiteral("QListView { border: %1px solid %2; border-radius: "
                                               "%3px; background: transparent; }")
                                    .arg(borderWidth)
                                    .arg(borderColor.name(QColor::HexArgb))
                                    .arg(borderRadius));
        listView->setAutoFillBackground(false);
        listView->viewport()->setAutoFillBackground(false);
        listView->viewport()->setAttribute(Qt::WA_OpaquePaintEvent, false);
        listView->setItemDelegate(
            new RoundedListItemDelegate(normalRowColor, alternateRowColor, listView));
        listView->setSpacing(Metrics::LIST_ROW_SPACING);
        listView->setUniformItemSizes(true);
    }
}

void configureListItem(QListWidgetItem *item)
{
    if (item == nullptr) {
        return;
    }

    item->setSizeHint(QSize(0, Metrics::LIST_ROW_HEIGHT));
}

void configureTableView(QTableView *view)
{
    configureItemView(view);
    view->horizontalHeader()->setStretchLastSection(true);
    view->verticalHeader()->setDefaultSectionSize(view->fontMetrics().height() +
                                                  (2 * Metrics::COMPACT_SPACING));
}

void fitTableViewToContents(QTableView *view)
{
    if (view == nullptr) {
        return;
    }

    const int tableContentHeight = view->horizontalHeader()->sizeHint().height() +
                                   view->verticalHeader()->length() + (2 * view->frameWidth());
    view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    view->setFixedHeight(tableContentHeight);
}

void configureMultilineField(QPlainTextEdit *field)
{
    configureTextControl(field);
    field->setMinimumHeight(Metrics::MULTILINE_EDITOR_MIN_HEIGHT);
    field->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void configureStatusView(QPlainTextEdit *view)
{
    configureDisplayTextControl(view);
    view->setReadOnly(true);
    view->setTextInteractionFlags(Qt::NoTextInteraction);
    view->setMaximumHeight(Metrics::STATUS_VIEW_HEIGHT);
}

void configureCodeStatusView(QPlainTextEdit *view)
{
    configureStatusView(view);
}

void configurePreviewSurface(QLabel *surface)
{
    surface->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    surface->setMinimumSize(Metrics::PREVIEW_MIN_SIZE, Metrics::PREVIEW_MIN_SIZE);
    surface->setFrameShape(QFrame::Box);
    surface->setAlignment(Qt::AlignCenter);
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

void configureSection(QFrame *section)
{
    section->setFrameShape(QFrame::StyledPanel);
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

void configureListActionButton(QAbstractButton *button, const QString &iconName,
                               const QString &toolTip)
{
    configureIconButton(button, iconName, toolTip);
    button->setFixedSize(Metrics::LIST_ACTION_BUTTON_SIZE, Metrics::LIST_ACTION_BUTTON_SIZE);
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

void configureCodeLabel(QLabel *label)
{
    label->setFont(standardFont());
}

void configureTextControl(QAbstractScrollArea *control)
{
    control->setFont(standardFont());
    const int borderWidth = textControlBorderWidth(control);
    int borderRadius = Metrics::CORNER_RADIUS;
    QColor borderColor = control->palette().color(QPalette::Mid);
    if (auto *const qlementineStyle =
            qobject_cast<oclero::qlementine::QlementineStyle *>(control->style())) {
        borderRadius = static_cast<int>(qlementineStyle->theme().borderRadius);
        borderColor = qlementineStyle->theme().borderColor;
    }
    control->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
    control->setLineWidth(borderWidth);
    setTextControlViewportMargins(control, borderWidth);
    control->setStyleSheet(
        QStringLiteral("QPlainTextEdit, QTextEdit { border: %1px solid %2; border-radius: %3px; }")
            .arg(borderWidth)
            .arg(borderColor.name(QColor::HexArgb))
            .arg(borderRadius));
    control->viewport()->setAutoFillBackground(false);
    control->viewport()->setAttribute(Qt::WA_OpaquePaintEvent, false);
}

void configureDisplayTextControl(QAbstractScrollArea *control)
{
    configureTextControl(control);
    const int borderWidth = textControlBorderWidth(control);
    control->setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
    control->setLineWidth(borderWidth);
    control->setFocusPolicy(Qt::NoFocus);
    control->setAttribute(Qt::WA_Hover, false);
    control->setMouseTracking(false);
    control->viewport()->setFocusPolicy(Qt::NoFocus);
    control->viewport()->setAttribute(Qt::WA_Hover, false);
    control->viewport()->setMouseTracking(false);
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

QFont standardFont()
{
    return QFontDatabase::systemFont(QFontDatabase::GeneralFont);
}

void configureCodeEditor(QPlainTextEdit *editor)
{
    configureTextControl(editor);
    editor->setTabStopDistance(Metrics::CODE_TAB_STOP);
}

} // namespace DevTools::Ui
