#ifndef CENTEREDDELEGATE_H
#define CENTEREDDELEGATE_H

#include <QStyledItemDelegate>


class CenteredDelegate: public QStyledItemDelegate {
    Q_OBJECT
    typedef QStyledItemDelegate Super;

public:
    CenteredDelegate(QWidget *parent = NULL);
protected:
    void initStyleOption(QStyleOptionViewItem *option,
                                 const QModelIndex &index) const override;
};

#endif // CENTEREDDELEGATE_H
