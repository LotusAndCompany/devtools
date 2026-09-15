#include "features/framework/gui/design_system.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLineEdit>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSplitter>
#include <QTableView>
#include <QtTest>

namespace Test {
class TestDesignSystem : public QObject
{
    Q_OBJECT

private slots:
    void configuresTextControl();
    void configuresStandardFields();
    void configuresPaneAndDialogFooter();
    void configuresItemViews();
    void configuresActionBarsAndSplitters();
    void configuresStatusView();
    void configuresExpandingSurface();
};

void TestDesignSystem::configuresTextControl()
{
    QPlainTextEdit editor;
    DevTools::Ui::configureTextControl(&editor);

    QCOMPARE(editor.frameShape(), QFrame::StyledPanel);
    QCOMPARE(editor.frameShadow(), QFrame::Raised);
    QCOMPARE(DevTools::Ui::previewContentSize(QSize(100, 100)), QSize(80, 80));
    QCOMPARE(DevTools::Ui::previewContentSize(QSize(10, 10)), QSize(1, 1));
}

void TestDesignSystem::configuresStandardFields()
{
    QLineEdit lineEdit;
    DevTools::Ui::configureLineEdit(&lineEdit);
    QCOMPARE(lineEdit.minimumWidth(), DevTools::Ui::Metrics::FIELD_MIN_WIDTH);
    QCOMPARE(lineEdit.sizePolicy().verticalPolicy(), QSizePolicy::Fixed);

    QComboBox comboBox;
    DevTools::Ui::configureComboBox(&comboBox);
    QCOMPARE(comboBox.minimumWidth(), DevTools::Ui::Metrics::FIELD_MIN_WIDTH);
    QCOMPARE(comboBox.sizePolicy().verticalPolicy(), QSizePolicy::Fixed);
}

void TestDesignSystem::configuresPaneAndDialogFooter()
{
    auto *const pane = DevTools::Ui::createPane(QStringLiteral("Pane"));
    QCOMPARE(pane->title(), QStringLiteral("Pane"));
    QCOMPARE(pane->sizePolicy().horizontalPolicy(), QSizePolicy::Expanding);
    DevTools::Ui::configureCompactPane(pane);
    QCOMPARE(pane->sizePolicy().verticalPolicy(), QSizePolicy::Maximum);
    delete pane;

    QDialogButtonBox buttonBox;
    DevTools::Ui::configureDialogButtonBox(&buttonBox);
    QVERIFY(!buttonBox.centerButtons());
    QCOMPARE(buttonBox.sizePolicy().verticalPolicy(), QSizePolicy::Fixed);
}

void TestDesignSystem::configuresItemViews()
{
    QListWidget listWidget;
    DevTools::Ui::configureItemView(&listWidget);
    QVERIFY(listWidget.alternatingRowColors());
    QCOMPARE(listWidget.selectionBehavior(), QAbstractItemView::SelectRows);

    QTableView tableView;
    DevTools::Ui::configureTableView(&tableView);
    QVERIFY(tableView.alternatingRowColors());
    QVERIFY(tableView.horizontalHeader()->stretchLastSection());
}

void TestDesignSystem::configuresActionBarsAndSplitters()
{
    QHBoxLayout leadingBar;
    DevTools::Ui::configureActionBar(&leadingBar, DevTools::Ui::ActionBarAlignment::Leading);
    QCOMPARE(leadingBar.contentsMargins(), DevTools::Ui::toolbarMargins());
    QCOMPARE(leadingBar.spacing(), DevTools::Ui::Metrics::CONTROL_SPACING);

    QHBoxLayout trailingBar;
    QPushButton trailingButton;
    trailingBar.addWidget(&trailingButton);
    DevTools::Ui::configureActionBar(&trailingBar, DevTools::Ui::ActionBarAlignment::Trailing);
    QCOMPARE(trailingBar.count(), 2);
    QVERIFY(trailingBar.itemAt(0)->spacerItem() != nullptr);

    QSplitter equalSplitter;
    equalSplitter.addWidget(new QWidget);
    equalSplitter.addWidget(new QWidget);
    DevTools::Ui::configureEqualSplitter(&equalSplitter);
    QCOMPARE(equalSplitter.handleWidth(), DevTools::Ui::Metrics::SPLITTER_HANDLE_WIDTH);
    QCOMPARE(equalSplitter.count(), 2);

    QSplitter mainSideSplitter;
    mainSideSplitter.addWidget(new QWidget);
    mainSideSplitter.addWidget(new QWidget);
    DevTools::Ui::configureMainSideSplitter(&mainSideSplitter);
    QCOMPARE(mainSideSplitter.handleWidth(), DevTools::Ui::Metrics::SPLITTER_HANDLE_WIDTH);

    QSplitter sideMainSplitter;
    sideMainSplitter.addWidget(new QWidget);
    sideMainSplitter.addWidget(new QWidget);
    DevTools::Ui::configureSideMainSplitter(&sideMainSplitter);
    QCOMPARE(sideMainSplitter.handleWidth(), DevTools::Ui::Metrics::SPLITTER_HANDLE_WIDTH);
}

void TestDesignSystem::configuresStatusView()
{
    QPlainTextEdit statusView;
    DevTools::Ui::configureStatusView(&statusView);

    QVERIFY(statusView.isReadOnly());
    QCOMPARE(statusView.maximumHeight(), DevTools::Ui::Metrics::STATUS_VIEW_HEIGHT);
    QCOMPARE(statusView.textInteractionFlags(),
             Qt::TextSelectableByKeyboard | Qt::TextSelectableByMouse);
}

void TestDesignSystem::configuresExpandingSurface()
{
    QWidget surface;
    DevTools::Ui::configureExpandingSurface(&surface);

    QCOMPARE(surface.sizePolicy().horizontalPolicy(), QSizePolicy::Expanding);
    QCOMPARE(surface.sizePolicy().verticalPolicy(), QSizePolicy::Expanding);
}
} // namespace Test

QTEST_MAIN(Test::TestDesignSystem)

#include "test_design_system.moc"
