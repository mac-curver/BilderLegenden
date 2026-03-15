#ifndef MULTILINEDELEGATE_H
#define MULTILINEDELEGATE_H

#include <QStyledItemDelegate>
#include <QPlainTextEdit>


class PropagatingTableView;

class MultiLineDelegate : public QStyledItemDelegate {
    Q_OBJECT
    typedef QStyledItemDelegate Super;

public:
    MultiLineDelegate(QWidget *parent = NULL);

    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    virtual void setEditorData(QWidget *editor, const QModelIndex &index) const;
    virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

    virtual void setTableView(PropagatingTableView *tableView);

private:
    PropagatingTableView *tableView = NULL;

};

#endif // MULTILINEDELEGATE_H
