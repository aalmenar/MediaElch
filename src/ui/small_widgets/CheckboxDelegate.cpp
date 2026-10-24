#include "ui/small_widgets/CheckboxDelegate.h"

#include <QApplication>
#include <QPainter>
#include <QStyle>
#include <QStyleOptionViewItem>

CheckboxDelegate::CheckboxDelegate(QObject* parent) : QStyledItemDelegate(parent)
{
}

void CheckboxDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    // Draw the item as usual.  This also keeps the layout (and therefore the
    // option's text rect) identical to Qt's default delegate.
    QStyledItemDelegate::paint(painter, option, index);

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    if (!(opt.features & QStyleOptionViewItem::HasCheckIndicator)) {
        return;
    }
    if (!(index.flags() & Qt::ItemIsEnabled)) {
        opt.state &= ~QStyle::State_Enabled;
    }

    QStyle* style = opt.widget != nullptr ? opt.widget->style() : QApplication::style();
    const QRect indicatorRect = style->subElementRect(QStyle::SE_ItemViewItemCheckIndicator, &opt, opt.widget);
    if (indicatorRect.isEmpty()) {
        return;
    }

    painter->save();
    painter->drawPixmap(indicatorRect, provideIndicatorPixmap(opt, style, indicatorRect.size()));
    painter->restore();
}

QPixmap CheckboxDelegate::provideIndicatorPixmap(const QStyleOptionViewItem& option, QStyle* style, QSize size) const
{
    const qreal devicePixelRatio = option.widget != nullptr ? option.widget->devicePixelRatioF() : 1.0;
    const QString key = QStringLiteral("%1x%2@%3/%4/%5/%6")
                            .arg(size.width())
                            .arg(size.height())
                            .arg(devicePixelRatio)
                            .arg(static_cast<int>(option.checkState))
                            .arg(option.state.testFlag(QStyle::State_Enabled) ? 1 : 0)
                            .arg(option.palette.cacheKey());

    const auto cacheEntry = m_cache.constFind(key);
    if (cacheEntry != m_cache.constEnd()) {
        return cacheEntry.value();
    }

    // Draw into an offscreen pixmap: platform styles that use native controls
    // only draw the first indicator of a paint pass.  A fresh painter per
    // indicator avoids that limitation.
    QPixmap pixmap(QSize(qRound(size.width() * devicePixelRatio), qRound(size.height() * devicePixelRatio)));
    pixmap.setDevicePixelRatio(devicePixelRatio);
    pixmap.fill(Qt::transparent);
    {
        QPainter painter(&pixmap);
        QStyleOptionViewItem opt = option;
        opt.rect = QRect(QPoint(0, 0), size);
        // Some styles (e.g. the macOS style) draw the check box using
        // State_On/State_Off instead of QStyleOptionViewItem::checkState.
        opt.state &= ~(QStyle::State_On | QStyle::State_Off | QStyle::State_NoChange);
        opt.state |= QStyle::State_Off;
        if (option.checkState == Qt::Checked) {
            opt.state |= QStyle::State_On;
            opt.state &= ~QStyle::State_Off;
        } else if (option.checkState == Qt::PartiallyChecked) {
            opt.state |= QStyle::State_NoChange;
        }
        style->drawPrimitive(QStyle::PE_IndicatorItemViewItemCheck, &opt, &painter, option.widget);
    }
    m_cache.insert(key, pixmap);
    return pixmap;
}
