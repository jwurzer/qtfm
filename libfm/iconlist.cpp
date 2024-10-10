#include "iconlist.h"

#include <QPainterPath>
#include <QEvent>

// list view / compact view

namespace {
    // spacing is 2 with list->setSpacing(2); --> half spacing is 1
    constexpr int HALF_SPACING = 1;
    // MARGIN_ITEM_... is an additional margin which is part of the item!
    //constexpr int MARGIN_ITEM_TOP = 0;
    //constexpr int MARGIN_ITEM_BOTTOM = 0;
    constexpr int MARGIN_ITEM_TOP = -HALF_SPACING;
    constexpr int MARGIN_ITEM_BOTTOM = -HALF_SPACING;
    constexpr int MARGIN_ITEM_LEFT = 2;
    constexpr int MARGIN_ITEM_RIGHT = 4; // margin after the text
    // margin between icon and text
    constexpr int MARGIN_BETWEEN_ICON_TEXT = 5;

    // margin for selection. this margin is removed from the item size
    // if no setSpacing(2) is used then TOP, LEFT, RIGHT should be set to 1
    constexpr int MARGIN_SELECT_TOP = 0;
    constexpr int MARGIN_SELECT_BOTTOM = 0; // 0 --> only the MARGIN_SELECT_TOP is between two entries
    constexpr int MARGIN_SELECT_LEFT = 0;
    constexpr int MARGIN_SELECT_RIGHT = 0;
}

bool IconListDelegate::eventFilter(QObject *object,
                                   QEvent *event)
{
    QWidget *editor = qobject_cast<QWidget*>(object);
    if(editor && event->type() == QEvent::KeyPress) {
        if(static_cast<QKeyEvent *>(event)->key() == Qt::Key_Escape){
            _isEditing = false;
            _index = QModelIndex();
        }
    }
    return QStyledItemDelegate::eventFilter(editor, event);
}

void IconListDelegate::setEditorData(QWidget *editor,
                                     const QModelIndex &index) const
{ // workaround for QTBUG
    _isEditing = true;
    _index = index;
    QStyledItemDelegate::setEditorData(editor, index);
}

void IconListDelegate::setModelData(QWidget *editor,
                                    QAbstractItemModel *model,
                                    const QModelIndex &index) const
{ // workaround for QTBUG
    QStyledItemDelegate::setModelData(editor, model, index);
    _isEditing = false;
    _index = QModelIndex();
}

QSize IconListDelegate::sizeHint(const QStyleOptionViewItem &option,
                                 const QModelIndex &index) const
{
    //QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
    //QSize iconsize = icon.actualSize(option.decorationSize);
    // don't use iconsize.width() and don't use iconsize.height() because of 1px hack for edit()
    //QSize iconsize = icon.actualSize(option.decorationSize);
    //int iconsizeWidth = iconsize.width();
    //int iconsizeHeight = iconsize.height();
    // instead using _iconsizeWidth and _iconsizeHeight
    int iconsizeWidth = _iconsizeWidth;
    int iconsizeHeight = _iconsizeHeight;
#if 1
    QRect item = option.rect;
    QRect txtRect(item.left() + iconsizeWidth + MARGIN_BETWEEN_ICON_TEXT,
                  item.top(), item.width(), item.height());
    QSize txtsize = option.fontMetrics.boundingRect(txtRect,
                                                    Qt::AlignLeft|Qt::AlignVCenter,
                                                    index.data().toString()).size();
#else
    QString text = index.data().toString();
    // Calculate the size based on the text
    QSize txtsize = QFontMetrics(option.font).size(Qt::TextSingleLine, text);
    //qInfo() << "size for text: '" << text << "' is " << txtsize;
#endif
    return QSize(
            MARGIN_ITEM_LEFT + iconsizeWidth + MARGIN_BETWEEN_ICON_TEXT + txtsize.width() + MARGIN_ITEM_RIGHT,
            MARGIN_ITEM_TOP + iconsizeHeight + MARGIN_ITEM_BOTTOM);
}

void IconListDelegate::paint(QPainter *painter,
                             const QStyleOptionViewItem &option,
                             const QModelIndex &index) const
{
    QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
    // don't use iconsize.width() and don't use iconsize.height() because of 1px hack for edit()
    //QSize iconsize = icon.actualSize(option.decorationSize);
    //int iconsizeWidth = iconsize.width();
    //int iconsizeHeight = iconsize.height();
    // instead using _iconsizeWidth and _iconsizeHeight
    int iconsizeWidth = _iconsizeWidth;
    int iconsizeHeight = _iconsizeHeight;
    QRect item = option.rect;
    QRect iconRect(item.left() + MARGIN_ITEM_LEFT, item.top() + MARGIN_ITEM_TOP, iconsizeWidth, iconsizeHeight);
    int txtShiftX = MARGIN_ITEM_LEFT + iconsizeWidth + MARGIN_BETWEEN_ICON_TEXT;
    QRect txtRect(item.left() + txtShiftX,
                  item.top() + MARGIN_ITEM_TOP,
                  item.width() - txtShiftX - MARGIN_ITEM_RIGHT,
                  item.height() - MARGIN_ITEM_TOP - MARGIN_ITEM_BOTTOM);
    QBrush txtBrush = qvariant_cast<QBrush>(index.data(Qt::ForegroundRole));
    bool isSelected = option.state & QStyle::State_Selected;
    bool isEditing = _isEditing && index==_index;

    painter->setRenderHint(QPainter::Antialiasing);

    if (isSelected && !isEditing) {
        QPainterPath path;
        QRect frame(
            item.left() + MARGIN_SELECT_LEFT,
            item.top() + MARGIN_SELECT_TOP,
            item.width() - MARGIN_SELECT_LEFT - MARGIN_SELECT_RIGHT,
            item.height() - MARGIN_SELECT_TOP - MARGIN_SELECT_BOTTOM);
        path.addRoundedRect(frame, 5, 5);
        painter->setOpacity(0.7);
        painter->fillPath(path, option.palette.highlight());
        painter->setOpacity(1.0);
    }

    painter->drawPixmap(iconRect, icon.pixmap(iconsizeWidth,iconsizeHeight));

    if (isEditing) { return; }
    if (isSelected) { painter->setPen(option.palette.highlightedText().color()); }
    else { painter->setPen(txtBrush.color()); }

    painter->drawText(txtRect, Qt::AlignLeft|Qt::AlignVCenter, index.data().toString());
}
