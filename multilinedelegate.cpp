#include "multilinedelegate.h"
//#include "svgdisplay.h"

MultiLineDelegate::MultiLineDelegate(QWidget *parent)
    : QStyledItemDelegate(parent)
{
}

QWidget *MultiLineDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    Q_UNUSED(option)
    Q_UNUSED(index)
    QPlainTextEdit *editor = new QPlainTextEdit(parent);
    return editor;
}

void MultiLineDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
    QPlainTextEdit *mEditor = qobject_cast<QPlainTextEdit *>(editor);
    mEditor->setPlainText(index.data().toString());
}

void MultiLineDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
    QPlainTextEdit *mEditor = qobject_cast<QPlainTextEdit *>(editor);
    model->setData(index, mEditor->toPlainText());
    if (printPreviewDialog) {
        printPreviewDialog->updatePreview();
    }
}


void MultiLineDelegate::setPreviewDialog(CustomPrintPreview *dlg) {
    printPreviewDialog = dlg;
}

