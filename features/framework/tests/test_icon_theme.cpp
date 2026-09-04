#include "features/framework/gui/icon_utils.h"
#include "features/framework/gui/sidemenu.h"

#include <QApplication>
#include <QColor>
#include <QIcon>
#include <QImage>
#include <QPalette>
#include <QPixmap>
#include <QSize>
#include <QStringList>
#include <QtTest>

namespace Test {
namespace {
QColor firstOpaqueColor(const QPixmap &pixmap)
{
    const QImage image = pixmap.toImage();
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            const QColor color = image.pixelColor(x, y);
            if (color.alpha() > 200) {
                return color;
            }
        }
    }

    return {};
}
} // namespace

class TestIconTheme : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    static void resolvesMaterialSymbols();
    static void resolvesHttpSidemenuIcon();
    static void fallsBackForMissingGlyph();
    static void followsApplicationPalette();
};

void TestIconTheme::initTestCase()
{
    QVERIFY(IconUtils::initializeMaterialSymbols());
    QCOMPARE(QIcon::themeName(), QStringLiteral("Material Symbols Outlined"));
}

void TestIconTheme::resolvesMaterialSymbols()
{
    const QStringList iconNames = {
        QStringLiteral("add"),
        QStringLiteral("article"),
        QStringLiteral("close"),
        QStringLiteral("content_copy"),
        QStringLiteral("content_paste"),
        QStringLiteral("database"),
        QStringLiteral("delete"),
        QStringLiteral("file_open"),
        QStringLiteral("flip_to_front"),
        QStringLiteral("image"),
        QStringLiteral("lan"),
        QStringLiteral("left_panel_close"),
        QStringLiteral("left_panel_open"),
        QStringLiteral("library_books"),
        QStringLiteral("link"),
        QStringLiteral("qr_code"),
        QStringLiteral("refresh"),
        QStringLiteral("rotate_left"),
        QStringLiteral("rotate_right"),
        QStringLiteral("save"),
        QStringLiteral("swap_horiz"),
        QStringLiteral("swap_vert"),
        QStringLiteral("terminal"),
        QStringLiteral("transform"),
        QStringLiteral("zoom_in"),
        QStringLiteral("zoom_out"),
    };

    for (const QString &name : iconNames) {
        QVERIFY2(QIcon::hasThemeIcon(name),
                 qPrintable(QStringLiteral("Missing icon: %1").arg(name)));

        const QIcon icon = QIcon::fromTheme(name);
        QVERIFY2(!icon.isNull(), qPrintable(QStringLiteral("Null icon: %1").arg(name)));
        const QPixmap pixmap = icon.pixmap(QSize(24, 24));
        QVERIFY2(!pixmap.isNull(),
                 qPrintable(QStringLiteral("Could not render icon: %1").arg(name)));
        QVERIFY2(firstOpaqueColor(pixmap).isValid(),
                 qPrintable(QStringLiteral("Could not render icon: %1").arg(name)));
    }
}

void TestIconTheme::resolvesHttpSidemenuIcon()
{
    const QIcon icon = Sidemenu::icon(Sidemenu::ID::HTTP_REQUEST);

    QCOMPARE(icon.name(), QStringLiteral("lan"));
    QVERIFY(!icon.isNull());
    QVERIFY(firstOpaqueColor(icon.pixmap(QSize(20, 20))).isValid());
}

void TestIconTheme::fallsBackForMissingGlyph()
{
    const QIcon icon = IconUtils::themedIcon(QStringLiteral("missing_material_symbol"));

    QVERIFY(!icon.isNull());
    QVERIFY(!icon.pixmap(QSize(24, 24)).isNull());
}

void TestIconTheme::followsApplicationPalette()
{
    const QPalette originalPalette = QApplication::palette();

    QPalette lightPalette = originalPalette;
    lightPalette.setColor(QPalette::Text, Qt::black);
    lightPalette.setColor(QPalette::Window, Qt::white);
    QApplication::setPalette(lightPalette);
    IconUtils::refreshMaterialSymbolsTheme();

    const QIcon icon = QIcon::fromTheme(QStringLiteral("home"));
    const QColor lightColor = firstOpaqueColor(icon.pixmap(QSize(24, 24)));

    QPalette darkPalette = originalPalette;
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Window, Qt::black);
    QApplication::setPalette(darkPalette);
    IconUtils::refreshMaterialSymbolsTheme();

    const QIcon darkIcon = QIcon::fromTheme(QStringLiteral("home"));
    const QColor darkColor = firstOpaqueColor(darkIcon.pixmap(QSize(24, 24)));

    QApplication::setPalette(originalPalette);
    IconUtils::refreshMaterialSymbolsTheme();

    QVERIFY(lightColor.isValid());
    QVERIFY(darkColor.isValid());
    QVERIFY(lightColor.lightness() < darkColor.lightness());
}
} // namespace Test

QTEST_MAIN(Test::TestIconTheme)

#include "test_icon_theme.moc"
