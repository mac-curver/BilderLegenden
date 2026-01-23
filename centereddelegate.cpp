#include "centereddelegate.h"


CenteredDelegate::CenteredDelegate(QWidget *parent): Super(parent) {

}

void CenteredDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const {
    Super::initStyleOption(option, index);
    option->displayAlignment = Qt::AlignCenter;
    option->viewItemPosition = QStyleOptionViewItem::Middle;
}
