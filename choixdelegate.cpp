#include "choixdelegate.h"

#include <QPainter>

ChoixDelegate::ChoixDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void ChoixDelegate::paint(QPainter *painter,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const
{
    const int choix = index.data().toInt();
    QString label;
    QColor color;
    switch (choix) {
    case 1:
        label = QStringLiteral("1er•90°");
        color = QColor(QStringLiteral("#27ae60"));
        break;
    case 2:
        label = QStringLiteral("2ème•180°");
        color = QColor(QStringLiteral("#e67e22"));
        break;
    default:
        label = QStringLiteral("3ème");
        color = QColor(QStringLiteral("#95a5a6"));
        break;
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setBrush(color);
    painter->setPen(Qt::NoPen);
    const QRect r = option.rect.adjusted(6, 4, -6, -4);
    painter->drawRoundedRect(r, 6, 6);
    painter->setPen(Qt::white);
    QFont f = option.font;
    f.setBold(true);
    f.setPointSize(8);
    painter->setFont(f);
    painter->drawText(r, Qt::AlignCenter, label);
    painter->restore();
}
