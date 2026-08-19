#include "features/framework/gui/design_system.h"

#include <QFormLayout>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSplitter>
#include <QTextBrowser>
#include <QToolBar>
#include <QVBoxLayout>
#include <QtTest>

#include <oclero/qlementine/Common.hpp>
#include <oclero/qlementine/widgets/Label.hpp>

namespace Test {
class TestDesignSystem : public QObject
{
    Q_OBJECT

private slots:
    void testMetricValues();
    void testLayoutHelpers();
    void testPaneHeading();
    void testTextControls();
    void testWidgetHelpers();
};

void TestDesignSystem::testMetricValues()
{
    QCOMPARE(DevTools::Ui::Metrics::PAGE_MARGIN, 16);
    QCOMPARE(DevTools::Ui::Metrics::PANEL_MARGIN, 12);
    QCOMPARE(DevTools::Ui::Metrics::ICON_BUTTON_SIZE, 32);
    QCOMPARE(DevTools::Ui::Metrics::CODE_TAB_STOP, 20);
    QCOMPARE(DevTools::Ui::mainWindowSize(), QSize(1280, 720));
    QCOMPARE(DevTools::Ui::dialogSize(), QSize(640, 480));
}

void TestDesignSystem::testLayoutHelpers()
{
    QWidget parent;
    QVBoxLayout layout(&parent);

    DevTools::Ui::applyPanelLayout(&layout);

    QCOMPARE(layout.contentsMargins(), DevTools::Ui::panelMargins());
    QCOMPARE(layout.spacing(), DevTools::Ui::Metrics::CONTROL_SPACING);

    DevTools::Ui::applyFullBleedLayout(&layout);
    QCOMPARE(layout.contentsMargins(), QMargins(0, 0, 0, 0));
    QCOMPARE(layout.spacing(), 0);

    QGridLayout metadataGrid;
    DevTools::Ui::configureCaptionValueGrid(&metadataGrid);
    QCOMPARE(metadataGrid.columnStretch(0), DevTools::Ui::Metrics::CAPTION_COLUMN_STRETCH);
    QCOMPARE(metadataGrid.columnStretch(1), DevTools::Ui::Metrics::VALUE_COLUMN_STRETCH);

    QToolBar toolbar;
    DevTools::Ui::configureTextToolbar(&toolbar);
    QCOMPARE(toolbar.toolButtonStyle(), Qt::ToolButtonTextOnly);

    QHBoxLayout equalLayout;
    QWidget equalFirst;
    QWidget equalSecond;
    equalLayout.addWidget(&equalFirst);
    equalLayout.addWidget(&equalSecond);
    DevTools::Ui::configureEqualLayout(&equalLayout);
    QCOMPARE(equalLayout.stretch(0), DevTools::Ui::Metrics::EQUAL_SPLIT_STRETCH);
    QCOMPARE(equalLayout.stretch(1), DevTools::Ui::Metrics::EQUAL_SPLIT_STRETCH);

    QHBoxLayout mainSideLayout;
    QWidget mainPanel;
    QWidget sidePanel;
    mainSideLayout.addWidget(&mainPanel);
    mainSideLayout.addWidget(&sidePanel);
    DevTools::Ui::configureMainSideLayout(&mainSideLayout);
    QCOMPARE(mainSideLayout.stretch(0), DevTools::Ui::Metrics::MAIN_PANEL_STRETCH);
    QCOMPARE(mainSideLayout.stretch(1), DevTools::Ui::Metrics::SIDE_PANEL_STRETCH);

    QHBoxLayout actionBar;
    DevTools::Ui::configureActionBar(&actionBar, DevTools::Ui::ActionBarAlignment::Trailing);
    QCOMPARE(actionBar.stretch(0), 1);

    QWidget actionBarParent;
    actionBarParent.resize(480, 40);
    auto *actionBarLayout = new QHBoxLayout(&actionBarParent);
    DevTools::Ui::configureActionBar(actionBarLayout, DevTools::Ui::ActionBarAlignment::Trailing);
    auto *firstAction = new QPushButton(QStringLiteral("First"), &actionBarParent);
    auto *secondAction = new QPushButton(QStringLiteral("Second"), &actionBarParent);
    DevTools::Ui::configureCompactButton(firstAction);
    DevTools::Ui::configureCompactButton(secondAction);
    actionBarLayout->addWidget(firstAction);
    actionBarLayout->addWidget(secondAction);
    actionBarParent.layout()->activate();
    QVERIFY(firstAction->x() > actionBarParent.contentsMargins().left());
    QVERIFY(secondAction->geometry().right() <=
            actionBarParent.width() - actionBarParent.contentsMargins().right());

    QWidget gridParent;
    gridParent.resize(900, 100);
    auto *grid = new QGridLayout(&gridParent);
    auto *gridActionLayout = new QHBoxLayout;
    DevTools::Ui::configureActionBar(gridActionLayout, DevTools::Ui::ActionBarAlignment::Trailing);
    auto *gridFirstAction = new QPushButton(QStringLiteral("First"), &gridParent);
    auto *gridSecondAction = new QPushButton(QStringLiteral("Second"), &gridParent);
    DevTools::Ui::configureCompactButton(gridFirstAction);
    DevTools::Ui::configureCompactButton(gridSecondAction);
    gridActionLayout->addWidget(gridFirstAction);
    gridActionLayout->addWidget(gridSecondAction);
    grid->addLayout(gridActionLayout, 0, 0, 1, 2);
    gridParent.layout()->activate();
    QVERIFY(gridFirstAction->x() > gridParent.contentsMargins().left());

    QSplitter splitter;
    splitter.addWidget(new QWidget(&splitter));
    splitter.addWidget(new QWidget(&splitter));
    DevTools::Ui::configureEqualSplitter(&splitter);
    QCOMPARE(splitter.handleWidth(), DevTools::Ui::Metrics::SPLITTER_HANDLE_WIDTH);

    QGridLayout overlayLayout;
    QWidget overlay;
    DevTools::Ui::addBottomRightOverlay(&overlayLayout, &overlay);
    QCOMPARE(overlayLayout.itemAt(0)->alignment(), Qt::AlignRight | Qt::AlignBottom);
}

void TestDesignSystem::testPaneHeading()
{
    QWidget parent;
    QLabel *const heading = DevTools::Ui::createPaneHeading(QStringLiteral("Input"), &parent);
    auto *const qlementineHeading = qobject_cast<oclero::qlementine::Label *>(heading);

    QVERIFY(qlementineHeading != nullptr);
    QCOMPARE(qlementineHeading->role(), oclero::qlementine::TextRole::H5);
    QCOMPARE(heading->text(), QStringLiteral("Input"));
    QCOMPARE(heading->alignment(), Qt::AlignLeft | Qt::AlignVCenter);
}

void TestDesignSystem::testTextControls()
{
    QWidget parent;

    QLineEdit lineEdit;
    DevTools::Ui::configureTextControl(&lineEdit);
    QVERIFY(lineEdit.hasFrame());

    QPlainTextEdit plainTextEdit;
    DevTools::Ui::configureTextControl(&plainTextEdit);
    QCOMPARE(plainTextEdit.frameShape(), QFrame::StyledPanel);
    QCOMPARE(plainTextEdit.frameShadow(), QFrame::Raised);
    QVERIFY(plainTextEdit.lineWidth() > 0);

    QTextBrowser textBrowser;
    DevTools::Ui::configureTextControl(&textBrowser);
    QCOMPARE(textBrowser.frameShape(), QFrame::StyledPanel);
    QCOMPARE(textBrowser.frameShadow(), QFrame::Raised);
    QVERIFY(textBrowser.lineWidth() > 0);

    auto *const styledTextBrowser = DevTools::Ui::createTextBrowser(&parent);
    auto *const styledLineEdit = DevTools::Ui::createLineEdit(&parent);
    auto *const styledPlainTextEdit = DevTools::Ui::createPlainTextEdit(&parent);
    QVERIFY(styledTextBrowser != nullptr);
    QCOMPARE(styledLineEdit->parentWidget(), &parent);
    QCOMPARE(styledPlainTextEdit->parentWidget(), &parent);

    QPlainTextEdit statusView;
    DevTools::Ui::configureStatusView(&statusView);
    QVERIFY(statusView.isReadOnly());
}

void TestDesignSystem::testWidgetHelpers()
{
    QPushButton button;
    DevTools::Ui::configureIconButton(&button, QStringLiteral("refresh"),
                                      QStringLiteral("Refresh"));
    QCOMPARE(button.size(), QSize(DevTools::Ui::Metrics::ICON_BUTTON_SIZE,
                                  DevTools::Ui::Metrics::ICON_BUTTON_SIZE));
    QCOMPARE(button.toolTip(), QStringLiteral("Refresh"));

    QLabel errorLabel;
    DevTools::Ui::configureErrorLabel(&errorLabel);
    QCOMPARE(errorLabel.property("devtoolsStatus").toString(), QStringLiteral("error"));

    QPlainTextEdit editor;
    DevTools::Ui::configureCodeEditor(&editor);
    QCOMPARE(editor.tabStopDistance(), static_cast<qreal>(DevTools::Ui::Metrics::CODE_TAB_STOP));

    QLineEdit field;
    DevTools::Ui::configureFormField(&field);
    QCOMPARE(field.minimumWidth(), DevTools::Ui::Metrics::FIELD_MIN_WIDTH);

    QWidget formParent;
    QFormLayout form(&formParent);
    DevTools::Ui::configureFormLayout(&form);
    QCOMPARE(form.contentsMargins(), DevTools::Ui::panelMargins());

    QFormLayout inlineForm;
    DevTools::Ui::configureInlineFormLayout(&inlineForm);
    QCOMPARE(inlineForm.contentsMargins(), QMargins(0, 0, 0, 0));

    QLabel preview;
    DevTools::Ui::configurePreviewSurface(&preview);
    QCOMPARE(preview.minimumSize(), QSize(DevTools::Ui::Metrics::PREVIEW_MIN_SIZE,
                                          DevTools::Ui::Metrics::PREVIEW_MIN_SIZE));
    QCOMPARE(DevTools::Ui::previewContentSize(QSize(100, 100)), QSize(80, 80));

    QLabel centeredLabel;
    DevTools::Ui::configureCenteredLabel(&centeredLabel);
    QCOMPARE(centeredLabel.alignment(), Qt::AlignCenter);
}
} // namespace Test

QTEST_MAIN(Test::TestDesignSystem)

#include "test_design_system.moc"
