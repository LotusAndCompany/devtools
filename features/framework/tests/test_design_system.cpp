#include "features/framework/gui/design_system.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFile>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QJsonDocument>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSplitter>
#include <QStandardItemModel>
#include <QStringList>
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
    void configuresDisplayTextControl();
    void configuresExpandingSurface();
    void keepsInteractionColorsNeutral();
};

void TestDesignSystem::configuresTextControl()
{
    QPlainTextEdit editor;
    DevTools::Ui::configureTextControl(&editor);

    QCOMPARE(editor.font(), DevTools::Ui::standardFont());
    QCOMPARE(editor.frameShape(), QFrame::StyledPanel);
    QCOMPARE(editor.frameShadow(), QFrame::Raised);
    QVERIFY(editor.styleSheet().contains(QStringLiteral("QPlainTextEdit")));
    QCOMPARE(DevTools::Ui::previewContentSize(QSize(100, 100)), QSize(80, 80));
    QCOMPARE(DevTools::Ui::previewContentSize(QSize(10, 10)), QSize(1, 1));
}

void TestDesignSystem::configuresStandardFields()
{
    QLineEdit lineEdit;
    DevTools::Ui::configureLineEdit(&lineEdit);
    QCOMPARE(lineEdit.font(), DevTools::Ui::standardFont());
    QCOMPARE(lineEdit.minimumWidth(), DevTools::Ui::Metrics::FIELD_MIN_WIDTH);
    QCOMPARE(lineEdit.sizePolicy().verticalPolicy(), QSizePolicy::Fixed);

    QComboBox comboBox;
    DevTools::Ui::configureComboBox(&comboBox);
    QCOMPARE(comboBox.font(), DevTools::Ui::standardFont());
    QCOMPARE(comboBox.minimumWidth(), DevTools::Ui::Metrics::FIELD_MIN_WIDTH);
    QCOMPARE(comboBox.sizePolicy().verticalPolicy(), QSizePolicy::Fixed);

    QLineEdit codeLineEdit;
    DevTools::Ui::configureCodeLineEdit(&codeLineEdit);
    QCOMPARE(codeLineEdit.font(), DevTools::Ui::standardFont());

    QPlainTextEdit multilineField;
    DevTools::Ui::configureMultilineField(&multilineField);
    QCOMPARE(multilineField.font(), DevTools::Ui::standardFont());

    QPlainTextEdit codeEditor;
    DevTools::Ui::configureCodeEditor(&codeEditor);
    QCOMPARE(codeEditor.font(), DevTools::Ui::standardFont());
}

void TestDesignSystem::configuresPaneAndDialogFooter()
{
    auto *const pane = DevTools::Ui::createPane(QStringLiteral("Pane"));
    QCOMPARE(pane->title(), QStringLiteral("Pane"));
    QVERIFY(!pane->isFlat());
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
    QCOMPARE(listWidget.font(), DevTools::Ui::standardFont());
    QCOMPARE(listWidget.frameShape(), QFrame::StyledPanel);
    QCOMPARE(listWidget.frameShadow(), QFrame::Plain);
    QVERIFY(listWidget.lineWidth() > 0);
    QVERIFY(listWidget.styleSheet().contains(QStringLiteral("QListView")));
    QVERIFY(listWidget.styleSheet().contains(QStringLiteral("border-radius")));
    QVERIFY(listWidget.alternatingRowColors());
    QCOMPARE(listWidget.selectionBehavior(), QAbstractItemView::SelectRows);
    QCOMPARE(listWidget.selectionMode(), QAbstractItemView::SingleSelection);
    QCOMPARE(listWidget.verticalScrollMode(), QAbstractItemView::ScrollPerPixel);
    QCOMPARE(listWidget.iconSize(),
             QSize(DevTools::Ui::Metrics::ICON_SIZE, DevTools::Ui::Metrics::ICON_SIZE));
    QCOMPARE(listWidget.spacing(), DevTools::Ui::Metrics::LIST_ROW_SPACING);
    QVERIFY(listWidget.uniformItemSizes());

    auto *const listItem = new QListWidgetItem;
    DevTools::Ui::configureListItem(listItem);
    QCOMPARE(listItem->sizeHint(), QSize(0, DevTools::Ui::Metrics::LIST_ROW_HEIGHT));
    delete listItem;

    QPushButton listActionButton;
    DevTools::Ui::configureListActionButton(&listActionButton, QStringLiteral("delete"));
    QCOMPARE(listActionButton.size(), QSize(DevTools::Ui::Metrics::LIST_ACTION_BUTTON_SIZE,
                                            DevTools::Ui::Metrics::LIST_ACTION_BUTTON_SIZE));

    QTableView tableView;
    DevTools::Ui::configureTableView(&tableView);
    QCOMPARE(tableView.frameShape(), QFrame::NoFrame);
    QCOMPARE(tableView.lineWidth(), 0);
    QCOMPARE(tableView.midLineWidth(), 0);
    QVERIFY(!tableView.styleSheet().contains(QStringLiteral("QTableView")));
    QVERIFY(tableView.alternatingRowColors());
    QVERIFY(tableView.horizontalHeader()->stretchLastSection());

    QStandardItemModel model(2, 1);
    tableView.setModel(&model);
    DevTools::Ui::fitTableViewToContents(&tableView);
    QCOMPARE(tableView.sizePolicy().verticalPolicy(), QSizePolicy::Fixed);
    QVERIFY(tableView.height() > tableView.horizontalHeader()->sizeHint().height());
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
    QCOMPARE(statusView.frameShape(), QFrame::StyledPanel);
    QCOMPARE(statusView.frameShadow(), QFrame::Plain);
    QVERIFY(statusView.lineWidth() > 0);
    QCOMPARE(statusView.focusPolicy(), Qt::NoFocus);
    QCOMPARE(statusView.textInteractionFlags(), Qt::NoTextInteraction);
    QCOMPARE(statusView.maximumHeight(), DevTools::Ui::Metrics::STATUS_VIEW_HEIGHT);
}

void TestDesignSystem::configuresDisplayTextControl()
{
    QPlainTextEdit display;
    DevTools::Ui::configureDisplayTextControl(&display);

    QCOMPARE(display.font(), DevTools::Ui::standardFont());
    QCOMPARE(display.frameShape(), QFrame::StyledPanel);
    QCOMPARE(display.frameShadow(), QFrame::Plain);
    QVERIFY(display.lineWidth() > 0);
    QCOMPARE(display.focusPolicy(), Qt::NoFocus);
    QCOMPARE(display.viewport()->focusPolicy(), Qt::NoFocus);
    QVERIFY(!display.testAttribute(Qt::WA_Hover));
    QVERIFY(!display.hasMouseTracking());
}

void TestDesignSystem::configuresExpandingSurface()
{
    QWidget surface;
    DevTools::Ui::configureExpandingSurface(&surface);

    QCOMPARE(surface.sizePolicy().horizontalPolicy(), QSizePolicy::Expanding);
    QCOMPARE(surface.sizePolicy().verticalPolicy(), QSizePolicy::Expanding);
}

void TestDesignSystem::keepsInteractionColorsNeutral()
{
    const QStringList interactionColorKeys{
        QStringLiteral("focusColor"),
        QStringLiteral("primaryColor"),
        QStringLiteral("primaryColorHovered"),
        QStringLiteral("primaryColorPressed"),
        QStringLiteral("primaryColorDisabled"),
        QStringLiteral("primaryAlternativeColor"),
        QStringLiteral("primaryAlternativeColorHovered"),
        QStringLiteral("primaryAlternativeColorPressed"),
        QStringLiteral("primaryAlternativeColorDisabled"),
        QStringLiteral("primaryColorForegroundDisabled"),
        QStringLiteral("statusColorInfo"),
        QStringLiteral("statusColorInfoHovered"),
        QStringLiteral("statusColorInfoPressed"),
        QStringLiteral("statusColorInfoDisabled"),
    };

    for (const QString &themeName : {QStringLiteral("light"), QStringLiteral("dark")}) {
        QFile themeFile(QStringLiteral(":/themes/%1.json").arg(themeName));
        QVERIFY2(themeFile.open(QIODevice::ReadOnly), qPrintable(themeFile.fileName()));

        QJsonParseError parseError;
        const QJsonDocument theme = QJsonDocument::fromJson(themeFile.readAll(), &parseError);
        QCOMPARE(parseError.error, QJsonParseError::NoError);

        for (const QString &key : interactionColorKeys) {
            QString colorValue = theme.object().value(key).toString();
            if (colorValue.size() == 9) {
                colorValue.chop(2);
            }
            const QColor color(colorValue);
            QVERIFY2(color.isValid(), qPrintable(key));

            const int maxChannelDelta =
                qMax(qAbs(color.red() - color.green()),
                     qMax(qAbs(color.green() - color.blue()), qAbs(color.blue() - color.red())));
            QVERIFY2(maxChannelDelta <= 8,
                     qPrintable(QStringLiteral("%1 in %2").arg(key, themeName)));
        }
    }
}
} // namespace Test

QTEST_MAIN(Test::TestDesignSystem)

#include "test_design_system.moc"
