#include "features/qr_code/gui/qr_code_generation_gui.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTimeZone>
#include <QtTest>

namespace {
QLineEdit *visibleLineEdit(QWidget &widget, const QString &placeholder)
{
    for (auto *edit : widget.findChildren<QLineEdit *>()) {
        if (edit->isVisible() && edit->placeholderText() == placeholder) {
            return edit;
        }
    }
    return nullptr;
}

QPlainTextEdit *visiblePlainTextEdit(QWidget &widget, const QString &placeholder)
{
    for (auto *edit : widget.findChildren<QPlainTextEdit *>()) {
        if (edit->isVisible() && edit->placeholderText() == placeholder) {
            return edit;
        }
    }
    return nullptr;
}

QComboBox *visibleComboBox(QWidget &widget, int itemCount)
{
    for (auto *combo : widget.findChildren<QComboBox *>()) {
        if (combo->isVisible() && combo->count() == itemCount) {
            return combo;
        }
    }
    return nullptr;
}
} // namespace

class TestQRGui : public QObject
{
    Q_OBJECT

private slots:
    void allTypesAndValidation();
};

void TestQRGui::allTypesAndValidation()
{
    QRCodeGenerationGUI widget;
    widget.resize(900, 600);
    widget.show();
    QCoreApplication::processEvents();

    QComboBox *category = visibleComboBox(widget, 9);
    QPushButton *generate = nullptr;
    QPushButton *copy = nullptr;
    for (auto *button : widget.findChildren<QPushButton *>()) {
        if (button->text() == "Generate") {
            generate = button;
        }
        if (button->text() == "Copy Image") {
            copy = button;
        }
    }
    QPlainTextEdit *preview = nullptr;
    for (auto *edit : widget.findChildren<QPlainTextEdit *>()) {
        if (edit->placeholderText() == "QR code content preview") {
            preview = edit;
        }
    }
    QVERIFY(category != nullptr);
    QVERIFY(generate != nullptr);
    QVERIFY(copy != nullptr);
    QVERIFY(preview != nullptr);

    category->setCurrentIndex(1);
    QLineEdit *url = visibleLineEdit(widget, "Enter URL...");
    QVERIFY(url != nullptr);
    QVERIFY(!generate->isEnabled());
    url->setText("bad-url");
    QVERIFY(generate->isEnabled());
    generate->click();
    bool found_error = false;
    for (const auto *label : widget.findChildren<QLabel *>()) {
        found_error |= label->text() == "URL must start with http:// or https://";
    }
    QVERIFY(found_error);
    QVERIFY(!copy->isEnabled());
    url->setText("https://example.com");
    generate->click();
    QCOMPARE(preview->toPlainText(), QString("https://example.com"));

    category->setCurrentIndex(0);
    QPlainTextEdit *text = visiblePlainTextEdit(widget, "Enter text...");
    QVERIFY(text != nullptr);
    text->setPlainText("Hello from UI");
    generate->click();
    QCOMPARE(preview->toPlainText(), QString("Hello from UI"));
    QVERIFY(copy->isEnabled());

    category->setCurrentIndex(2);
    QLineEdit *email = visibleLineEdit(widget, "Enter email address...");
    QVERIFY(email != nullptr);
    email->setText("a@example.com");
    generate->click();
    QCOMPARE(preview->toPlainText(), QString("mailto:a@example.com"));

    category->setCurrentIndex(3);
    QLineEdit *phone = visibleLineEdit(widget, "Enter phone number...");
    QVERIFY(phone != nullptr);
    phone->setText("09012345678");
    generate->click();
    QCOMPARE(preview->toPlainText(), QString("tel:09012345678"));

    category->setCurrentIndex(4);
    phone = visibleLineEdit(widget, "Enter phone number...");
    QPlainTextEdit *message = visiblePlainTextEdit(widget, "Enter message...");
    QVERIFY(phone != nullptr);
    QVERIFY(message != nullptr);
    phone->setText("09012345678");
    message->setPlainText("Hello");
    generate->click();
    QCOMPARE(preview->toPlainText(), QString("SMSTO:09012345678:Hello"));

    category->setCurrentIndex(5);
    QLineEdit *ssid = visibleLineEdit(widget, "Enter SSID...");
    QComboBox *security = visibleComboBox(widget, 3);
    QVERIFY(ssid != nullptr);
    QVERIFY(security != nullptr);
    ssid->setText("Guest");
    security->setCurrentIndex(2);
    for (auto *checkBox : widget.findChildren<QCheckBox *>()) {
        if (checkBox->isVisible()) {
            checkBox->setChecked(true);
        }
    }
    generate->click();
    QCOMPARE(preview->toPlainText(), QString("WIFI:T:nopass;S:Guest;P:;;"));

    category->setCurrentIndex(6);
    QLineEdit *name = visibleLineEdit(widget, "Enter name...");
    QLineEdit *contactPhone = visibleLineEdit(widget, "Enter phone...");
    QVERIFY(name != nullptr);
    QVERIFY(contactPhone != nullptr);
    name->setText("Alice");
    contactPhone->setText("09012345678");
    generate->click();
    QVERIFY(preview->toPlainText().contains("FN:Alice"));
    QVERIFY(preview->toPlainText().contains("TEL:09012345678"));

    category->setCurrentIndex(7);
    QLineEdit *summary = visibleLineEdit(widget, "Enter event title...");
    const auto dates = widget.findChildren<QDateTimeEdit *>();
    QVERIFY(summary != nullptr);
    QCOMPARE(dates.size(), 2);
    const QDateTime time(QDate(2026, 2, 3), QTime(12, 0), QTimeZone::utc());
    dates[0]->setDateTime(time);
    dates[1]->setDateTime(time);
    summary->setText("Meeting");
    generate->click();
    found_error = false;
    for (const auto *label : widget.findChildren<QLabel *>()) {
        found_error |= label->text() == "End time must be after start time";
    }
    QVERIFY(found_error);
    dates[1]->setDateTime(time.addSecs(3600));
    generate->click();
    QVERIFY(preview->toPlainText().contains("SUMMARY:Meeting"));

    category->setCurrentIndex(8);
    QLineEdit *latitude = visibleLineEdit(widget, "Enter latitude...");
    QLineEdit *longitude = visibleLineEdit(widget, "Enter longitude...");
    QVERIFY(latitude != nullptr);
    QVERIFY(longitude != nullptr);
    latitude->setText("-90");
    longitude->setText("180");
    generate->click();
    QCOMPARE(preview->toPlainText(), QString("geo:-90.000000,180.000000"));

    bool found_image = false;
    for (const auto *label : widget.findChildren<QLabel *>()) {
        found_image |= !label->pixmap().isNull();
    }
    QVERIFY(found_image);
}

QTEST_MAIN(TestQRGui)
#include "test_qr_gui.moc"
