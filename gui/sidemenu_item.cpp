#include "sidemenu_item.h"

#include "core/enum_cast.h"
#include "core/exception/invalid_argument_exception.h"
#include "core/tool/tool.h"

#include <QMouseEvent>
#include <QPainter>
#include <QStyleOptionButton>

const QString SidemenuItem::notConfigurableReason = "Sidemenu::ID::HOME is not confugurable";

SidemenuItem::SidemenuItem(Sidemenu::ID id, QWidget *parent) : QPushButton(parent), id(id)
{
    setFlat(true);
    setCheckable(true);
    setIconSize(QSize(20, 20));
    setFocusPolicy(Qt::FocusPolicy::NoFocus);

    Sidemenu::validateID(id);

    if (id == Sidemenu::ID::WELCOME) {
        throw InvalidArgumentException(static_cast<int>(id), notConfigurableReason);
    }

    setIcon(Sidemenu::icon(id));
    // NOTE: Tool::IDとSidemenu::IDはHOME, MAX以外は一致する想定
    setText(Tool::translatable(enum_cast<Tool::ID>(static_cast<int>(id))).name);
}

void SidemenuItem::changeEvent(QEvent *event)
{
    switch (event->type()) {
    case QEvent::LanguageChange:
        // アイコンは変更しない想定

        // NOTE: Tool::IDとSidemenu::IDはHOME, MAX以外は一致する想定
        setText(Tool::translatable(enum_cast<Tool::ID>(static_cast<int>(id))).name);
        event->accept();
        break;
    default:
        QPushButton::changeEvent(event);
        break;
    }
}

void SidemenuItem::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);

    QStyleOptionButton option;
    initStyleOption(&option);

    // ベベルは qlementine に描画させる
    style()->drawControl(QStyle::CE_PushButtonBevel, &option, &painter, this);

    constexpr int LEFT_MARGIN = 8;
    constexpr int ICON_TEXT_SPACING = 6;
    constexpr int RIGHT_MARGIN = 8;

    int contentX = option.rect.x() + LEFT_MARGIN;

    if (!option.icon.isNull()) {
        const auto iconMode =
            (option.state & QStyle::State_Enabled) != 0 ? QIcon::Normal : QIcon::Disabled;
        const auto iconState = (option.state & QStyle::State_On) != 0 ? QIcon::On : QIcon::Off;
        const auto iconSz = iconSize();
        QRect const logicalIconRect(
            contentX, option.rect.y() + ((option.rect.height() - iconSz.height()) / 2),
            iconSz.width(), iconSz.height());
        QRect const iconRect = QStyle::visualRect(option.direction, option.rect, logicalIconRect);
        option.icon.paint(&painter, iconRect, Qt::AlignCenter, iconMode, iconState);
        contentX += iconSz.width() + ICON_TEXT_SPACING;
    }

    int const textMaxWidth = option.rect.width() - contentX - RIGHT_MARGIN;
    if (textMaxWidth > 0 && !option.text.isEmpty()) {
        QRect const logicalTextRect(contentX, option.rect.y(), textMaxWidth, option.rect.height());
        QRect const textRect = QStyle::visualRect(option.direction, option.rect, logicalTextRect);

        // qlementine のテーマカラーを palette から使う
        // WindowText = secondaryColor, HighlightedText = primaryColorForeground
        auto const textColor = (option.state & QStyle::State_On) != 0
                                   ? palette().color(QPalette::HighlightedText)
                                   : palette().color(QPalette::WindowText);
        painter.setPen(textColor);

        QString const elidedText =
            fontMetrics().elidedText(option.text, Qt::ElideRight, textMaxWidth, Qt::TextSingleLine);
        const auto textAlign =
            QStyle::visualAlignment(option.direction, Qt::AlignLeft | Qt::AlignVCenter);
        painter.drawText(textRect, static_cast<int>(textAlign) | Qt::TextSingleLine, elidedText);
    }
}
