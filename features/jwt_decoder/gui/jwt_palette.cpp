#include "jwt_palette.h"

#include <QApplication>
#include <QStyle>

#include <oclero/qlementine/style/QlementineStyle.hpp>
#include <oclero/qlementine/style/Theme.hpp>

JwtPalette JwtPalette::current()
{
    JwtPalette palette;

    // 素の QApplication で動くテストなどでは QlementineStyle が入っていない
    const auto *style =
        qobject_cast<const oclero::qlementine::QlementineStyle *>(QApplication::style());
    if (style == nullptr) {
        return palette;
    }

    const oclero::qlementine::Theme &theme = style->theme();

    palette.segmentSignature = theme.statusColorInfo;
    palette.jsonKey = theme.primaryAlternativeColor;
    palette.jsonString = theme.statusColorSuccess;
    palette.errorText = theme.statusColorError;
    palette.errorBackground = theme.statusColorErrorDisabled;
    palette.noticeBackground = theme.statusColorWarningDisabled;
    palette.noticeText = theme.secondaryColor;
    palette.secondaryText = theme.secondaryAlternativeColor;

    return palette;
}

bool JwtPalette::operator==(const JwtPalette &other) const
{
    return segmentHeader == other.segmentHeader && segmentPayload == other.segmentPayload &&
           segmentSignature == other.segmentSignature && jsonKey == other.jsonKey &&
           jsonString == other.jsonString && jsonNumber == other.jsonNumber &&
           errorText == other.errorText && errorBackground == other.errorBackground &&
           noticeBackground == other.noticeBackground && noticeText == other.noticeText &&
           secondaryText == other.secondaryText;
}
