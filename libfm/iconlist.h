/*
# Copyright (c) 2018, Ole-André Rodlie <ole.andre.rodlie@gmail.com> All rights reserved.
#
# Available under the 3-clause BSD license
# See the LICENSE file for full details
*/

#include <QObject>
#include <QItemDelegate>
#include <QStyledItemDelegate>
#include <QKeyEvent>
#include <QModelIndex>
#include <QPainter>

// list view / compact view

class IconListDelegate :  public QStyledItemDelegate
{
private:
    // workaround for edit() hack
    int _iconsizeWidth = 1;
    int _iconsizeHeight = 1;
    // workaround for QTBUG
    mutable bool _isEditing;
    mutable QModelIndex _index;
protected: // workaround for QTBUG
    bool eventFilter(QObject * object,
                     QEvent * event);
public:
    void setIconSize(int width, int height) { _iconsizeWidth = width; _iconsizeHeight = height; }
    void setEditorData(QWidget * editor,
                       const QModelIndex & index) const;
    void setModelData(QWidget * editor,
                      QAbstractItemModel * model,
                      const QModelIndex & index) const;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const;
    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const;
};
