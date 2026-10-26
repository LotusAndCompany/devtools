#include "timestamp_conversion_gui.h"

#include "features/framework/gui/icon_utils.h"
#include "features/timestamp_conversion/core/timestamp_conversion.h"

#include <QApplication>
#include <QClipboard>
#include <QDateTimeEdit>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTimeZone>
#include <QVBoxLayout>

namespace {
constexpr int DEFAULT_WIDTH = 720;
constexpr int DEFAULT_HEIGHT = 360;
const char *const STATUS_LABEL_STYLE = "color: red;";
const char *const HINT_LABEL_STYLE = "color: gray;";
const char *const DATE_TIME_DISPLAY_FORMAT = "yyyy-MM-dd hh:mm:ss";
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

    convertButton = new QPushButton(this);
    convertButton->setIcon(IconUtils::themedIcon(QStringLiteral("refresh")));

    copyButton = new QPushButton(this);
    copyButton->setIcon(IconUtils::themedIcon(QStringLiteral("content_copy")));

    grid->addWidget(label, row, 0);
    grid->addWidget(editor, row, 1);
    grid->addWidget(convertButton, row, 2);
    grid->addWidget(copyButton, row, 3);
}

QGroupBox *TimestampConversionGUI::buildUnixTimestampGroup()
{
    unixGroupBox = new QGroupBox(this);
    auto *const grid = new QGridLayout(unixGroupBox);
    grid->setColumnStretch(1, 1);

    int row = 0;

    secondsEdit = new QLineEdit(this);
    addRow(grid, row++, secondsLabel, secondsEdit, secondsConvertButton, secondsCopyButton);

    millisecondsEdit = new QLineEdit(this);
    addRow(grid, row++, millisecondsLabel, millisecondsEdit, millisecondsConvertButton,
           millisecondsCopyButton);

    return unixGroupBox;
}

QGroupBox *TimestampConversionGUI::buildDateTimeGroup()
{
    dateTimeGroupBox = new QGroupBox(this);
    auto *const grid = new QGridLayout(dateTimeGroupBox);
    grid->setColumnStretch(1, 1);

    int row = 0;

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

    return dateTimeGroupBox;
}

QGroupBox *TimestampConversionGUI::buildIso8601Group()
{
    isoGroupBox = new QGroupBox(this);
    auto *const grid = new QGridLayout(isoGroupBox);
    grid->setColumnStretch(1, 1);

    int row = 0;

    isoUtcEdit = new QLineEdit(this);
    addRow(grid, row++, isoUtcLabel, isoUtcEdit, isoUtcConvertButton, isoUtcCopyButton);

    isoLocalEdit = new QLineEdit(this);
    addRow(grid, row++, isoLocalLabel, isoLocalEdit, isoLocalConvertButton, isoLocalCopyButton);

    return isoGroupBox;
}

void TimestampConversionGUI::buildUi()
{
    resize(DEFAULT_WIDTH, DEFAULT_HEIGHT);

    auto *const mainLayout = new QVBoxLayout(this);

    auto *const topBarLayout = new QHBoxLayout();
    hintLabel = new QLabel(this);
    hintLabel->setWordWrap(true);
    hintLabel->setStyleSheet(HINT_LABEL_STYLE);
    topBarLayout->addWidget(hintLabel, 1);
    nowButton = new QPushButton(this);
    nowButton->setIcon(IconUtils::themedIcon(QStringLiteral("schedule")));
    topBarLayout->addWidget(nowButton);
    mainLayout->addLayout(topBarLayout);

    mainLayout->addWidget(buildUnixTimestampGroup());
    mainLayout->addWidget(buildDateTimeGroup());
    mainLayout->addWidget(buildIso8601Group());
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

    hintLabel->setText(
        tr("Enter a value in any row below, then press Convert to apply it to every other row."));
    nowButton->setText(tr("Now"));

    unixGroupBox->setTitle(tr("Unix Timestamp"));
    secondsLabel->setText(tr("Seconds"));
    secondsEdit->setPlaceholderText(tr("e.g. 1700000000"));
    secondsConvertButton->setText(tr("Convert"));
    secondsConvertButton->setToolTip(tr("Convert from this value"));
    secondsCopyButton->setText(tr("Copy"));
    secondsCopyButton->setToolTip(tr("Copy"));

    millisecondsLabel->setText(tr("Milliseconds"));
    millisecondsEdit->setPlaceholderText(tr("e.g. 1700000000000"));
    millisecondsConvertButton->setText(tr("Convert"));
    millisecondsConvertButton->setToolTip(tr("Convert from this value"));
    millisecondsCopyButton->setText(tr("Copy"));
    millisecondsCopyButton->setToolTip(tr("Copy"));

    dateTimeGroupBox->setTitle(tr("Date && Time"));
    localLabel->setText(tr("Local"));
    localConvertButton->setText(tr("Convert"));
    localConvertButton->setToolTip(tr("Convert from this value"));
    localCopyButton->setText(tr("Copy"));
    localCopyButton->setToolTip(tr("Copy"));

    utcLabel->setText(tr("UTC"));
    utcConvertButton->setText(tr("Convert"));
    utcConvertButton->setToolTip(tr("Convert from this value"));
    utcCopyButton->setText(tr("Copy"));
    utcCopyButton->setToolTip(tr("Copy"));

    isoGroupBox->setTitle(tr("ISO 8601"));
    isoUtcLabel->setText(tr("UTC"));
    isoUtcEdit->setPlaceholderText(tr("e.g. 2026-09-13T20:32:37.497Z"));
    isoUtcConvertButton->setText(tr("Convert"));
    isoUtcConvertButton->setToolTip(tr("Convert from this value"));
    isoUtcCopyButton->setText(tr("Copy"));
    isoUtcCopyButton->setToolTip(tr("Copy"));

    isoLocalLabel->setText(tr("Local"));
    isoLocalEdit->setPlaceholderText(tr("e.g. 2026-09-14T05:32:37.497+09:00"));
    isoLocalConvertButton->setText(tr("Convert"));
    isoLocalConvertButton->setToolTip(tr("Convert from this value"));
    isoLocalCopyButton->setText(tr("Copy"));
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
