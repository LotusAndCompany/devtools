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

    connect(nowButton, &QPushButton::clicked, this, &TimestampConversionGUI::onNowClicked);

    connect(secondsConvertButton, &QPushButton::clicked, this,
            &TimestampConversionGUI::onConvertFromSecondsClicked);
    connect(millisecondsConvertButton, &QPushButton::clicked, this,
            &TimestampConversionGUI::onConvertFromMillisecondsClicked);
    connect(localConvertButton, &QPushButton::clicked, this,
            &TimestampConversionGUI::onConvertFromLocalClicked);
    connect(utcConvertButton, &QPushButton::clicked, this,
            &TimestampConversionGUI::onConvertFromUtcClicked);
    connect(isoUtcConvertButton, &QPushButton::clicked, this,
            &TimestampConversionGUI::onConvertFromIsoUtcClicked);
    connect(isoLocalConvertButton, &QPushButton::clicked, this,
            &TimestampConversionGUI::onConvertFromIsoLocalClicked);

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

    secondsLabel = new QLabel(this);
    secondsEdit = new QLineEdit(this);
    secondsConvertButton = createIconButton(QStringLiteral("view-refresh"), this);
    secondsCopyButton = createIconButton(QStringLiteral("edit-copy"), this);
    grid->addWidget(secondsLabel, row, 0);
    grid->addWidget(secondsEdit, row, 1);
    grid->addWidget(secondsConvertButton, row, 2);
    grid->addWidget(secondsCopyButton, row, 3);
    ++row;

    millisecondsLabel = new QLabel(this);
    millisecondsEdit = new QLineEdit(this);
    millisecondsConvertButton = createIconButton(QStringLiteral("view-refresh"), this);
    millisecondsCopyButton = createIconButton(QStringLiteral("edit-copy"), this);
    grid->addWidget(millisecondsLabel, row, 0);
    grid->addWidget(millisecondsEdit, row, 1);
    grid->addWidget(millisecondsConvertButton, row, 2);
    grid->addWidget(millisecondsCopyButton, row, 3);
    ++row;

    localLabel = new QLabel(this);
    localEdit = new QDateTimeEdit(this);
    localEdit->setDisplayFormat(DATE_TIME_DISPLAY_FORMAT);
    localEdit->setCalendarPopup(true);
    localConvertButton = createIconButton(QStringLiteral("view-refresh"), this);
    localCopyButton = createIconButton(QStringLiteral("edit-copy"), this);
    grid->addWidget(localLabel, row, 0);
    grid->addWidget(localEdit, row, 1);
    grid->addWidget(localConvertButton, row, 2);
    grid->addWidget(localCopyButton, row, 3);
    ++row;

    utcLabel = new QLabel(this);
    utcEdit = new QDateTimeEdit(this);
    utcEdit->setDisplayFormat(DATE_TIME_DISPLAY_FORMAT);
    utcEdit->setCalendarPopup(true);
    utcConvertButton = createIconButton(QStringLiteral("view-refresh"), this);
    utcCopyButton = createIconButton(QStringLiteral("edit-copy"), this);
    grid->addWidget(utcLabel, row, 0);
    grid->addWidget(utcEdit, row, 1);
    grid->addWidget(utcConvertButton, row, 2);
    grid->addWidget(utcCopyButton, row, 3);
    ++row;

    isoUtcLabel = new QLabel(this);
    isoUtcEdit = new QLineEdit(this);
    isoUtcConvertButton = createIconButton(QStringLiteral("view-refresh"), this);
    isoUtcCopyButton = createIconButton(QStringLiteral("edit-copy"), this);
    grid->addWidget(isoUtcLabel, row, 0);
    grid->addWidget(isoUtcEdit, row, 1);
    grid->addWidget(isoUtcConvertButton, row, 2);
    grid->addWidget(isoUtcCopyButton, row, 3);
    ++row;

    isoLocalLabel = new QLabel(this);
    isoLocalEdit = new QLineEdit(this);
    isoLocalConvertButton = createIconButton(QStringLiteral("view-refresh"), this);
    isoLocalCopyButton = createIconButton(QStringLiteral("edit-copy"), this);
    grid->addWidget(isoLocalLabel, row, 0);
    grid->addWidget(isoLocalEdit, row, 1);
    grid->addWidget(isoLocalConvertButton, row, 2);
    grid->addWidget(isoLocalCopyButton, row, 3);
    ++row;

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
    secondsEdit->setText(
        TimestampConversion::toUnixTimestamp(utcInstant, TimestampConversion::Unit::Seconds));
    millisecondsEdit->setText(
        TimestampConversion::toUnixTimestamp(utcInstant, TimestampConversion::Unit::Milliseconds));

    // QDateTimeEditはローカルタイムゾーンの日時として値を表示するため、
    // UTC欄には見た目上の年月日時分秒だけを渡す（timeSpecはQDateTimeEdit内部で
    // ローカル扱いされる。読み戻す際はfromUtcDateTime()がUTCとして再解釈する）
    const QDateTime localWallClock = TimestampConversion::toLocalDateTime(utcInstant);
    localEdit->setDateTime(QDateTime(localWallClock.date(), localWallClock.time()));
    utcEdit->setDateTime(QDateTime(utcInstant.date(), utcInstant.time()));

    isoUtcEdit->setText(TimestampConversion::toIso8601(utcInstant));
    isoLocalEdit->setText(TimestampConversion::toIso8601Local(utcInstant));

    statusLabel->clear();
}

void TimestampConversionGUI::showInvalidValueError()
{
    statusLabel->setText(tr("Invalid value. Please check the input and try again."));
}

void TimestampConversionGUI::onNowClicked()
{
    applyCanonical(QDateTime::currentDateTimeUtc());
}

void TimestampConversionGUI::onConvertFromSecondsClicked()
{
    bool ok = false;
    const QDateTime utcInstant = TimestampConversion::fromUnixTimestamp(
        secondsEdit->text(), TimestampConversion::Unit::Seconds, &ok);

    if (!ok) {
        showInvalidValueError();
        return;
    }
    applyCanonical(utcInstant);
}

void TimestampConversionGUI::onConvertFromMillisecondsClicked()
{
    bool ok = false;
    const QDateTime utcInstant = TimestampConversion::fromUnixTimestamp(
        millisecondsEdit->text(), TimestampConversion::Unit::Milliseconds, &ok);

    if (!ok) {
        showInvalidValueError();
        return;
    }
    applyCanonical(utcInstant);
}

void TimestampConversionGUI::onConvertFromLocalClicked()
{
    const QDateTime utcInstant = TimestampConversion::fromLocalDateTime(localEdit->dateTime());

    if (!utcInstant.isValid()) {
        showInvalidValueError();
        return;
    }
    applyCanonical(utcInstant);
}

void TimestampConversionGUI::onConvertFromUtcClicked()
{
    const QDateTime utcInstant = TimestampConversion::fromUtcDateTime(utcEdit->dateTime());

    if (!utcInstant.isValid()) {
        showInvalidValueError();
        return;
    }
    applyCanonical(utcInstant);
}

void TimestampConversionGUI::onConvertFromIsoUtcClicked()
{
    bool ok = false;
    const QDateTime utcInstant = TimestampConversion::fromIso8601(isoUtcEdit->text(), &ok);

    if (!ok) {
        showInvalidValueError();
        return;
    }
    applyCanonical(utcInstant);
}

void TimestampConversionGUI::onConvertFromIsoLocalClicked()
{
    bool ok = false;
    const QDateTime utcInstant = TimestampConversion::fromIso8601(isoLocalEdit->text(), &ok);

    if (!ok) {
        showInvalidValueError();
        return;
    }
    applyCanonical(utcInstant);
}
