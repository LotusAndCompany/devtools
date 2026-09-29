#include "timestamp_conversion_gui.h"

#include "features/timestamp_conversion/core/timestamp_conversion.h"

#include <QApplication>
#include <QClipboard>
#include <QDateTimeEdit>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTimeZone>
#include <QVBoxLayout>

namespace {
constexpr int DEFAULT_WIDTH = 720;
constexpr int DEFAULT_HEIGHT = 280;
const char *const STATUS_LABEL_STYLE = "color: red;";
const char *const DATE_TIME_DISPLAY_FORMAT = "yyyy-MM-dd hh:mm:ss";

QPushButton *createIconButton(const QString &iconName, QWidget *parent)
{
    auto *const button = new QPushButton(parent);
    button->setIcon(QIcon::fromTheme(iconName));
    return button;
}
} // namespace

TimestampConversionGUI::TimestampConversionGUI(QWidget *parent) : GuiTool(parent)
{
    buildUi();

    connect(nowButton, &QPushButton::clicked, this,
            [this]() { applyCanonical(QDateTime::currentDateTimeUtc()); });

    connect(secondsConvertButton, &QPushButton::clicked, this, [this]() {
        applyCanonical(TimestampConversion::fromUnixTimestamp(secondsEdit->text(),
                                                              TimestampConversion::Unit::Seconds));
    });
    connect(millisecondsConvertButton, &QPushButton::clicked, this, [this]() {
        applyCanonical(TimestampConversion::fromUnixTimestamp(
            millisecondsEdit->text(), TimestampConversion::Unit::Milliseconds));
    });
    connect(localConvertButton, &QPushButton::clicked, this,
            [this]() { applyCanonical(localEdit->dateTime().toUTC()); });
    connect(utcConvertButton, &QPushButton::clicked, this,
            [this]() { applyCanonical(utcEdit->dateTime().toUTC()); });
    connect(isoUtcConvertButton, &QPushButton::clicked, this,
            [this]() { applyCanonical(TimestampConversion::fromIso8601(isoUtcEdit->text())); });
    connect(isoLocalConvertButton, &QPushButton::clicked, this,
            [this]() { applyCanonical(TimestampConversion::fromIso8601(isoLocalEdit->text())); });

    connect(secondsCopyButton, &QPushButton::clicked, this,
            [this]() { QApplication::clipboard()->setText(secondsEdit->text()); });
    connect(millisecondsCopyButton, &QPushButton::clicked, this,
            [this]() { QApplication::clipboard()->setText(millisecondsEdit->text()); });
    connect(localCopyButton, &QPushButton::clicked, this,
            [this]() { QApplication::clipboard()->setText(localEdit->text()); });
    connect(utcCopyButton, &QPushButton::clicked, this,
            [this]() { QApplication::clipboard()->setText(utcEdit->text()); });
    connect(isoUtcCopyButton, &QPushButton::clicked, this,
            [this]() { QApplication::clipboard()->setText(isoUtcEdit->text()); });
    connect(isoLocalCopyButton, &QPushButton::clicked, this,
            [this]() { QApplication::clipboard()->setText(isoLocalEdit->text()); });
}

void TimestampConversionGUI::addRow(QGridLayout *grid, int row, QLabel *&label, QWidget *editor,
                                    QPushButton *&convertButton, QPushButton *&copyButton)
{
    label = new QLabel(this);
    convertButton = createIconButton(QStringLiteral("view-refresh"), this);
    copyButton = createIconButton(QStringLiteral("edit-copy"), this);
    grid->addWidget(label, row, 0);
    grid->addWidget(editor, row, 1);
    grid->addWidget(convertButton, row, 2);
    grid->addWidget(copyButton, row, 3);
}

void TimestampConversionGUI::buildUi()
{
    resize(DEFAULT_WIDTH, DEFAULT_HEIGHT);

    auto *const mainLayout = new QVBoxLayout(this);

    auto *const topBarLayout = new QHBoxLayout();
    topBarLayout->addStretch();
    nowButton = new QPushButton(this);
    nowButton->setIcon(QIcon::fromTheme(QStringLiteral("clock")));
    topBarLayout->addWidget(nowButton);
    mainLayout->addLayout(topBarLayout);

    auto *const grid = new QGridLayout();
    grid->setColumnStretch(1, 1);

    int row = 0;

    secondsEdit = new QLineEdit(this);
    addRow(grid, row++, secondsLabel, secondsEdit, secondsConvertButton, secondsCopyButton);

    millisecondsEdit = new QLineEdit(this);
    addRow(grid, row++, millisecondsLabel, millisecondsEdit, millisecondsConvertButton,
           millisecondsCopyButton);

    // Qt 6.9以降のQDateTimeEdit::setTimeZone()を使い、表示・読み取りのタイムゾーン変換を
    // ウィジェット自身に任せる（coreは常にUTCの基準時刻だけを扱えばよい）
    localEdit = new QDateTimeEdit(this);
    localEdit->setDisplayFormat(DATE_TIME_DISPLAY_FORMAT);
    localEdit->setCalendarPopup(true);
    localEdit->setTimeZone(QTimeZone::systemTimeZone());
    addRow(grid, row++, localLabel, localEdit, localConvertButton, localCopyButton);

    utcEdit = new QDateTimeEdit(this);
    utcEdit->setDisplayFormat(DATE_TIME_DISPLAY_FORMAT);
    utcEdit->setCalendarPopup(true);
    utcEdit->setTimeZone(QTimeZone::UTC);
    addRow(grid, row++, utcLabel, utcEdit, utcConvertButton, utcCopyButton);

    isoUtcEdit = new QLineEdit(this);
    addRow(grid, row++, isoUtcLabel, isoUtcEdit, isoUtcConvertButton, isoUtcCopyButton);

    isoLocalEdit = new QLineEdit(this);
    addRow(grid, row++, isoLocalLabel, isoLocalEdit, isoLocalConvertButton, isoLocalCopyButton);

    mainLayout->addLayout(grid);
    mainLayout->addStretch();

    statusLabel = new QLabel(this);
    statusLabel->setStyleSheet(STATUS_LABEL_STYLE);
    statusLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(statusLabel);

    // 起動時は現在時刻を基準にして全行を揃えておく
    applyCanonical(QDateTime::currentDateTimeUtc());

    retranslateUi();
}

void TimestampConversionGUI::retranslateUi()
{
    setWindowTitle(tr("Timestamp Conversion"));

    nowButton->setText(tr("Now"));

    secondsLabel->setText(tr("Unix Timestamp (seconds)"));
    secondsEdit->setPlaceholderText(tr("e.g. 1700000000"));
    secondsConvertButton->setToolTip(tr("Convert from this value"));
    secondsCopyButton->setToolTip(tr("Copy"));

    millisecondsLabel->setText(tr("Unix Timestamp (milliseconds)"));
    millisecondsEdit->setPlaceholderText(tr("e.g. 1700000000000"));
    millisecondsConvertButton->setToolTip(tr("Convert from this value"));
    millisecondsCopyButton->setToolTip(tr("Copy"));

    localLabel->setText(tr("Date && Time (Local)"));
    localConvertButton->setToolTip(tr("Convert from this value"));
    localCopyButton->setToolTip(tr("Copy"));

    utcLabel->setText(tr("Date && Time (UTC)"));
    utcConvertButton->setToolTip(tr("Convert from this value"));
    utcCopyButton->setToolTip(tr("Copy"));

    isoUtcLabel->setText(tr("ISO 8601 (UTC)"));
    isoUtcEdit->setPlaceholderText(tr("e.g. 2026-09-13T20:32:37.497Z"));
    isoUtcConvertButton->setToolTip(tr("Convert from this value"));
    isoUtcCopyButton->setToolTip(tr("Copy"));

    isoLocalLabel->setText(tr("ISO 8601 (Local)"));
    isoLocalEdit->setPlaceholderText(tr("e.g. 2026-09-14T05:32:37.497+09:00"));
    isoLocalConvertButton->setToolTip(tr("Convert from this value"));
    isoLocalCopyButton->setToolTip(tr("Copy"));

    statusLabel->clear();
}

void TimestampConversionGUI::changeEvent(QEvent *event)
{
    switch (event->type()) {
    case QEvent::LanguageChange:
        retranslateUi();
        event->accept();
        break;
    default:
        QWidget::changeEvent(event);
        break;
    }
}

void TimestampConversionGUI::applyCanonical(const QDateTime &utcInstant)
{
    if (!utcInstant.isValid()) {
        statusLabel->setText(tr("Invalid value. Please check the input and try again."));
        return;
    }

    secondsEdit->setText(
        TimestampConversion::toUnixTimestamp(utcInstant, TimestampConversion::Unit::Seconds));
    millisecondsEdit->setText(
        TimestampConversion::toUnixTimestamp(utcInstant, TimestampConversion::Unit::Milliseconds));
    localEdit->setDateTime(utcInstant);
    utcEdit->setDateTime(utcInstant);
    isoUtcEdit->setText(TimestampConversion::toIso8601(utcInstant));
    isoLocalEdit->setText(TimestampConversion::toIso8601Local(utcInstant));

    statusLabel->clear();
}
