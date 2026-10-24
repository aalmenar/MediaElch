#pragma once

#include <QHash>
#include <QPixmap>
#include <QStyledItemDelegate>

/**
 * \brief Item delegate that reliably draws check box indicators.
 *
 * Some platform styles (e.g. Qt's macOS style, which draws check boxes using
 * native AppKit controls) do not draw more than one item view check indicator
 * per paint pass.  Every other check box of a table view is silently dropped,
 * i.e. a table whose check boxes are not in the first painted cell appears to
 * have no check boxes at all.
 *
 * This delegate lets QStyledItemDelegate paint the item as usual and then draws
 * the current style's check indicator itself, using an offscreen pixmap so that
 * the platform style starts from a clean painter.  Since painting is the only
 * thing that is changed, check state handling (clicking a check box) as well as
 * the item's text, icon and background stay untouched.
 */
class CheckboxDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit CheckboxDelegate(QObject* parent = nullptr);

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private:
    QPixmap provideIndicatorPixmap(const QStyleOptionViewItem& option, QStyle* style, QSize size) const;

    /// Cached indicators; the key contains all properties that affect the drawing.
    mutable QHash<QString, QPixmap> m_cache;
};
