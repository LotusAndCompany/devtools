#include "color_converter.h"

#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QtMath>

ColorConverter::ColorConverter(QObject *parent) : QObject(parent)
{
    addToHistory(m_currentColor);
}

// ==========================================================================
// HEX
// ==========================================================================

QString ColorConverter::toHex(const QColor &color, bool includeAlpha)
{
    return includeAlpha ? color.name(QColor::HexArgb).toUpper()
                         : color.name(QColor::HexRgb).toUpper();
}

QColor ColorConverter::fromHex(const QString &hex, bool *ok)
{
    QString normalized = hex.trimmed();
    if (!normalized.startsWith('#')) {
        normalized.prepend('#');
    }

    const QColor color(normalized);
    const bool valid = color.isValid();
    if (ok != nullptr) {
        *ok = valid;
    }

    return valid ? color : QColor();
}

// ==========================================================================
// RGB
// ==========================================================================

QString ColorConverter::toRgbString(const QColor &color)
{
    if (color.alpha() < 255) {
        return QString("rgba(%1, %2, %3, %4)")
            .arg(color.red())
            .arg(color.green())
            .arg(color.blue())
            .arg(QString::number(color.alphaF(), 'g', 2));
    }
    return QString("rgb(%1, %2, %3)").arg(color.red()).arg(color.green()).arg(color.blue());
}

QColor ColorConverter::fromRgbString(const QString &text, bool *ok)
{
    static const QRegularExpression re(
        R"(rgba?\(\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*(?:,\s*([\d.]+)\s*)?\))",
        QRegularExpression::CaseInsensitiveOption);

    const QRegularExpressionMatch match = re.match(text.trimmed());
    const bool valid = match.hasMatch();
    if (ok != nullptr) {
        *ok = valid;
    }
    if (!valid) {
        return QColor();
    }

    const int r = qBound(0, match.captured(1).toInt(), 255);
    const int g = qBound(0, match.captured(2).toInt(), 255);
    const int b = qBound(0, match.captured(3).toInt(), 255);
    const bool hasAlpha = !match.captured(4).isEmpty();
    const double a = hasAlpha ? qBound(0.0, match.captured(4).toDouble(), 1.0) : 1.0;

    QColor color(r, g, b);
    color.setAlphaF(static_cast<float>(a));
    return color;
}

// ==========================================================================
// HSL
// ==========================================================================

QString ColorConverter::toHslString(const QColor &color)
{
    const int h = qRound(color.hslHueF() < 0 ? 0.0 : color.hslHueF() * 360.0);
    const int s = qRound(color.hslSaturationF() * 100.0);
    const int l = qRound(color.lightnessF() * 100.0);
    return QString("hsl(%1, %2%, %3%)").arg(h).arg(s).arg(l);
}

QColor ColorConverter::fromHslString(const QString &text, bool *ok)
{
    static const QRegularExpression re(
        R"(hsla?\(\s*(\d+(?:\.\d+)?)\s*,\s*(\d+(?:\.\d+)?)%\s*,\s*(\d+(?:\.\d+)?)%\s*\))",
        QRegularExpression::CaseInsensitiveOption);

    const QRegularExpressionMatch match = re.match(text.trimmed());
    const bool valid = match.hasMatch();
    if (ok != nullptr) {
        *ok = valid;
    }
    if (!valid) {
        return QColor();
    }

    const double h = qBound(0.0, match.captured(1).toDouble(), 359.999);
    const double s = qBound(0.0, match.captured(2).toDouble(), 100.0) / 100.0;
    const double l = qBound(0.0, match.captured(3).toDouble(), 100.0) / 100.0;

    return QColor::fromHslF(static_cast<float>(h / 360.0), static_cast<float>(s),
                             static_cast<float>(l));
}

// ==========================================================================
// HSV
// ==========================================================================

QString ColorConverter::toHsvString(const QColor &color)
{
    const int h = qRound(color.hsvHueF() < 0 ? 0.0 : color.hsvHueF() * 360.0);
    const int s = qRound(color.hsvSaturationF() * 100.0);
    const int v = qRound(color.valueF() * 100.0);
    return QString("hsv(%1, %2%, %3%)").arg(h).arg(s).arg(v);
}

QColor ColorConverter::fromHsvString(const QString &text, bool *ok)
{
    static const QRegularExpression re(
        R"(hsva?\(\s*(\d+(?:\.\d+)?)\s*,\s*(\d+(?:\.\d+)?)%\s*,\s*(\d+(?:\.\d+)?)%\s*\))",
        QRegularExpression::CaseInsensitiveOption);

    const QRegularExpressionMatch match = re.match(text.trimmed());
    const bool valid = match.hasMatch();
    if (ok != nullptr) {
        *ok = valid;
    }
    if (!valid) {
        return QColor();
    }

    const double h = qBound(0.0, match.captured(1).toDouble(), 359.999);
    const double s = qBound(0.0, match.captured(2).toDouble(), 100.0) / 100.0;
    const double v = qBound(0.0, match.captured(3).toDouble(), 100.0) / 100.0;

    return QColor::fromHsvF(static_cast<float>(h / 360.0), static_cast<float>(s),
                             static_cast<float>(v));
}

// ==========================================================================
// CMYK
// ==========================================================================

QString ColorConverter::toCmykString(const QColor &color)
{
    const int c = qRound(color.cyanF() * 100.0);
    const int m = qRound(color.magentaF() * 100.0);
    const int y = qRound(color.yellowF() * 100.0);
    const int k = qRound(color.blackF() * 100.0);
    return QString("cmyk(%1%, %2%, %3%, %4%)").arg(c).arg(m).arg(y).arg(k);
}

QColor ColorConverter::fromCmykString(const QString &text, bool *ok)
{
    static const QRegularExpression re(
        R"(cmyk\(\s*(\d+(?:\.\d+)?)%\s*,\s*(\d+(?:\.\d+)?)%\s*,\s*(\d+(?:\.\d+)?)%\s*,\s*(\d+(?:\.\d+)?)%\s*\))",
        QRegularExpression::CaseInsensitiveOption);

    const QRegularExpressionMatch match = re.match(text.trimmed());
    const bool valid = match.hasMatch();
    if (ok != nullptr) {
        *ok = valid;
    }
    if (!valid) {
        return QColor();
    }

    const double c = qBound(0.0, match.captured(1).toDouble(), 100.0) / 100.0;
    const double m = qBound(0.0, match.captured(2).toDouble(), 100.0) / 100.0;
    const double y = qBound(0.0, match.captured(3).toDouble(), 100.0) / 100.0;
    const double k = qBound(0.0, match.captured(4).toDouble(), 100.0) / 100.0;

    return QColor::fromCmykF(static_cast<float>(c), static_cast<float>(m), static_cast<float>(y),
                              static_cast<float>(k));
}

// ==========================================================================
// 色差・コントラスト
// ==========================================================================

namespace {

/// sRGB(0-255)の1チャンネルをリニアRGBへ変換する
double srgbChannelToLinear(int channel8bit)
{
    const double c = channel8bit / 255.0;
    return (c <= 0.04045) ? (c / 12.92) : qPow((c + 0.055) / 1.055, 2.4);
}

/// QColor(sRGB)をCIE XYZ(D65基準)へ変換する
void colorToXyz(const QColor &color, double *x, double *y, double *z)
{
    const double r = srgbChannelToLinear(color.red());
    const double g = srgbChannelToLinear(color.green());
    const double b = srgbChannelToLinear(color.blue());

    // sRGB -> XYZ (D65)
    *x = r * 0.4124564 + g * 0.3575761 + b * 0.1804375;
    *y = r * 0.2126729 + g * 0.7151522 + b * 0.0721750;
    *z = r * 0.0193339 + g * 0.1191920 + b * 0.9503041;
}

/// CIE XYZをCIE Labへ変換する(D65白色点基準)
void xyzToLab(double x, double y, double z, double *l, double *a, double *b)
{
    // D65白色点
    constexpr double XN = 0.95047;
    constexpr double YN = 1.00000;
    constexpr double ZN = 1.08883;

    auto f = [](double t) {
        constexpr double DELTA = 6.0 / 29.0;
        if (t > DELTA * DELTA * DELTA) {
            return qPow(t, 1.0 / 3.0);
        }
        return t / (3.0 * DELTA * DELTA) + 4.0 / 29.0;
    };

    const double fx = f(x / XN);
    const double fy = f(y / YN);
    const double fz = f(z / ZN);

    *l = 116.0 * fy - 16.0;
    *a = 500.0 * (fx - fy);
    *b = 200.0 * (fy - fz);
}

} // namespace

double ColorConverter::deltaE76(const QColor &lhs, const QColor &rhs)
{
    double x1 = 0.0, y1 = 0.0, z1 = 0.0;
    double x2 = 0.0, y2 = 0.0, z2 = 0.0;
    colorToXyz(lhs, &x1, &y1, &z1);
    colorToXyz(rhs, &x2, &y2, &z2);

    double l1 = 0.0, a1 = 0.0, b1 = 0.0;
    double l2 = 0.0, a2 = 0.0, b2 = 0.0;
    xyzToLab(x1, y1, z1, &l1, &a1, &b1);
    xyzToLab(x2, y2, z2, &l2, &a2, &b2);

    return qSqrt(qPow(l1 - l2, 2.0) + qPow(a1 - a2, 2.0) + qPow(b1 - b2, 2.0));
}

double ColorConverter::relativeLuminance(const QColor &color)
{
    const double r = srgbChannelToLinear(color.red());
    const double g = srgbChannelToLinear(color.green());
    const double b = srgbChannelToLinear(color.blue());
    return 0.2126 * r + 0.7152 * g + 0.0722 * b;
}

double ColorConverter::contrastRatio(const QColor &lhs, const QColor &rhs)
{
    const double l1 = relativeLuminance(lhs);
    const double l2 = relativeLuminance(rhs);
    const double lighter = qMax(l1, l2);
    const double darker = qMin(l1, l2);
    return (lighter + 0.05) / (darker + 0.05);
}

ColorConverter::ContrastLevel ColorConverter::contrastLevel(double ratio)
{
    if (ratio >= 7.0) {
        return ContrastLevel::AAA;
    }
    if (ratio >= 4.5) {
        return ContrastLevel::AA;
    }
    if (ratio >= 3.0) {
        return ContrastLevel::AA_Large;
    }
    return ContrastLevel::Fail;
}

QString ColorConverter::contrastLevelLabel(ContrastLevel level)
{
    switch (level) {
    case ContrastLevel::AAA:
        return tr("AAA");
    case ContrastLevel::AA:
        return tr("AA");
    case ContrastLevel::AA_Large:
        return tr("AA (Large)");
    case ContrastLevel::Fail:
    default:
        return tr("Fail");
    }
}

// ==========================================================================
// インスタンスの状態
// ==========================================================================

void ColorConverter::setCurrentColor(const QColor &color)
{
    if (!color.isValid() || color == m_currentColor) {
        return;
    }

    m_currentColor = color;
    m_toColor = color;

    addToHistory(color);

    emit currentColorChanged(m_currentColor);
    emit comparisonChanged();
}

void ColorConverter::lockCurrentAsFrom()
{
    if (m_fromColor == m_currentColor) {
        return;
    }

    m_fromColor = m_currentColor;
    emit comparisonChanged();
}

void ColorConverter::swapFromTo()
{
    const QColor previousFrom = m_fromColor;
    m_fromColor = m_toColor;

    // 現在色を新しい「To」の値に更新する(履歴には追加しない)
    m_currentColor = previousFrom;
    m_toColor = previousFrom;

    emit currentColorChanged(m_currentColor);
    emit comparisonChanged();
}

void ColorConverter::addToHistory(const QColor &color)
{
    // 既に履歴にある場合は取り除いてから先頭に追加する(直近使用順)
    m_history.removeAll(color);
    m_history.prepend(color);

    while (m_history.size() > HISTORY_MAX_SIZE) {
        m_history.removeLast();
    }

    emit historyChanged();
}

void ColorConverter::clearHistory()
{
    if (m_history.isEmpty()) {
        return;
    }
    m_history.clear();
    emit historyChanged();
}
