#ifndef CHOIXDELEGATE_H
#define CHOIXDELEGATE_H

#include <QStyledItemDelegate>

class ChoixDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ChoixDelegate(QObject *parent = nullptr);
    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
};

#endif
