#include "icon_utils.h"

#include <QApplication>
#include <QFontDatabase>

namespace {
const QString MATERIAL_SYMBOLS_FONT_FAMILY = QStringLiteral("Material Symbols Outlined");
const QString MATERIAL_SYMBOLS_FONT_RESOURCE =
    QStringLiteral(":/fonts/MaterialSymbolsOutlined.ttf");
} // namespace

bool IconUtils::initializeMaterialSymbols()
{
    const int fontId = QFontDatabase::addApplicationFont(MATERIAL_SYMBOLS_FONT_RESOURCE);
    if (fontId < 0) {
        qWarning() << "Failed to load Material Symbols font:" << MATERIAL_SYMBOLS_FONT_RESOURCE;
        return false;
    }

    const QStringList families = QFontDatabase::applicationFontFamilies(fontId);
    if (!families.contains(MATERIAL_SYMBOLS_FONT_FAMILY)) {
        qWarning() << "Material Symbols font family not found. Families:" << families;
        QFontDatabase::removeApplicationFont(fontId);
        return false;
    }

    QIcon::setThemeName(MATERIAL_SYMBOLS_FONT_FAMILY);
    qDebug() << "Loaded Material Symbols font:" << MATERIAL_SYMBOLS_FONT_FAMILY;
    return true;
}

void IconUtils::refreshMaterialSymbolsTheme()
{
    // QFontIconEngine caches rendered pixmaps, so changing the theme key forces a repaint.
    QIcon::setThemeName(QString());
    QIcon::setThemeName(MATERIAL_SYMBOLS_FONT_FAMILY);
}

QIcon IconUtils::themedIcon(const QString &name, QStyle::StandardPixmap fallback)
{
    QIcon fallbackIcon = QApplication::style()->standardIcon(fallback);
    if (fallbackIcon.isNull()) {
        fallbackIcon = QApplication::style()->standardIcon(QStyle::SP_FileIcon);
    }

    return QIcon::fromTheme(name, fallbackIcon);
}
