#ifndef DESIGN_SYSTEM_H
#define DESIGN_SYSTEM_H

#include <QFont>
#include <QMargins>
#include <QSize>
#include <QString>

#include <cstdint>

class QAbstractButton;
class QAbstractScrollArea;
class QBoxLayout;
class QFormLayout;
class QFrame;
class QGridLayout;
class QLabel;
class QLayout;
class QPlainTextEdit;
class QPushButton;
class QScrollArea;
class QSplitter;
class QTextBrowser;
class QToolButton;
class QWidget;

namespace DevTools::Ui {

enum class ActionBarAlignment : std::uint8_t {
    Leading,
    Trailing,
};

namespace Metrics {
constexpr int PAGE_MARGIN = 16;
constexpr int PANEL_MARGIN = 12;
constexpr int TOOLBAR_MARGIN = 4;
constexpr int PAGE_SPACING = 12;
constexpr int CONTROL_SPACING = 8;
constexpr int COMPACT_SPACING = 4;
constexpr int ICON_BUTTON_SIZE = 32;
constexpr int ICON_SIZE = 20;
constexpr int STATUS_VIEW_HEIGHT = 64;
constexpr int SIDEMENU_WIDTH = 240;
constexpr int TOOL_PANEL_WIDTH = 360;
constexpr int FIELD_MIN_WIDTH = 200;
constexpr int MULTILINE_EDITOR_MIN_HEIGHT = 100;
constexpr int CODE_TAB_STOP = 20;
constexpr int PREVIEW_MIN_SIZE = 320;
constexpr int PREVIEW_INSET = 10;
constexpr int LOGO_SIZE = 64;
constexpr int HERO_POINT_SIZE = 24;
constexpr int TITLE_POINT_SIZE = 36;
constexpr int SPLITTER_HANDLE_WIDTH = 8;
constexpr int EQUAL_SPLIT_STRETCH = 1;
constexpr int SIDE_PANEL_STRETCH = 1;
constexpr int MAIN_PANEL_STRETCH = 3;
constexpr int CAPTION_COLUMN_STRETCH = 1;
constexpr int VALUE_COLUMN_STRETCH = 2;
constexpr double ZOOM_BUTTON_OPACITY = 0.8;
} // namespace Metrics

QSize mainWindowSize();
QSize dialogSize();

QMargins pageMargins();
QMargins panelMargins();
QMargins toolbarMargins();

void applyPageLayout(QLayout *layout);
void applyPanelLayout(QLayout *layout);
void applyToolbarLayout(QLayout *layout);
void applyInlineLayout(QLayout *layout);
void applyFullBleedLayout(QLayout *layout);
void configureFormLayout(QFormLayout *layout);
void configureInlineFormLayout(QFormLayout *layout);
void configureActionBar(QBoxLayout *layout, ActionBarAlignment alignment);
void configureEqualLayout(QBoxLayout *layout);
void configureEqualSplitter(QSplitter *splitter);
void configureCaptionValueGrid(QGridLayout *layout);
void addBottomRightOverlay(QGridLayout *layout, QWidget *overlay);

void configureDialog(QWidget *dialog);
QLabel *createPaneHeading(const QString &text, QWidget *parent);
void configurePaneHeading(QLabel *label);
void configureCenteredLabel(QLabel *label);
void configureSidebarItem(QPushButton *button);
void configurePrimaryButton(QAbstractButton *button);
void configureCompactButton(QAbstractButton *button);
void configureWindowControlButton(QPushButton *button, const QString &iconName,
                                  const QString &toolTip = {});
void configureDivider(QFrame *divider);
QTextBrowser *createTextBrowser(QWidget *parent = nullptr);
void configureFormField(QWidget *field);
void configureMultilineField(QPlainTextEdit *field);
void configureStatusView(QPlainTextEdit *view);
void configurePreviewSurface(QLabel *surface);
void configureImageSurface(QLabel *surface);
void configureExpandingSurface(QWidget *surface);
void configurePaneSurface(QWidget *surface);
void configureScrollView(QScrollArea *view);
void configureSection(QFrame *section);
void configureZoomButton(QToolButton *button, const QString &iconName, const QString &toolTip);
QLabel *createErrorLabel(QWidget *parent);
QSize previewContentSize(const QSize &surfaceSize);

void configureIconButton(QAbstractButton *button, const QString &iconName,
                         const QString &toolTip = {});
void configureErrorLabel(QLabel *label);
void refreshStatusColors();

void configureTextControl(QAbstractScrollArea *control);

QFont codeFont();
void configureCodeEditor(QPlainTextEdit *editor);

} // namespace DevTools::Ui

#endif // DESIGN_SYSTEM_H
