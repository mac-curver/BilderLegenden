#include <QDebug>
#include <QMimeData>
#include <QMessageBox>
#include <QPrintPreviewWidget>
#include <QFile>
//#include "logfile.h"
#include "customprintpreview.h"
#include "multilinedelegate.h"


#include "propagatingtableview.h"

PropagatingTableView::PropagatingTableView(QWidget *parent): QTableView(parent) {
    qDebug() << "PropagatingTableView::";
    multiLineDelegate = new MultiLineDelegate;
    setItemDelegateForColumn(RowType::Description, multiLineDelegate);

    centerDelegate = new CenteredDelegate;
    setItemDelegateForColumn(RowType::Image, centerDelegate);

    setAutoFillBackground(true);


    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setDefaultDropAction(Qt::MoveAction);

    setDragDropMode(QAbstractItemView::DragDrop);
    setDragDropOverwriteMode(false);


    setModel(&legendsModel);
    setWordWrap(true);
    setTextElideMode(Qt::ElideMiddle);

    codeWidget = new QrCodeWidget();
    printPreviewDialog = new CustomPrintPreview(codeWidget->printer, this);
    codeWidget->setImageLegends(static_cast<ImageLegendsModel*>(this->model()));
    connect(printPreviewDialog, &CustomPrintPreview::paintRequested, codeWidget, &QrCodeWidget::printPreview);

    resizeRowsToContents();
}

PropagatingTableView::~PropagatingTableView() {
    delete multiLineDelegate;
}

void PropagatingTableView::setColumnWidths() {
    setColumnWidth(RowType::ColumnType::Image,  70);
    setColumnWidth(RowType::ColumnType::Title, 150);
    setColumnWidth(RowType::ColumnType::Description, 250);
    setColumnWidth(RowType::ColumnType::Author, 150);
}

void PropagatingTableView::setPreviewDialog(CustomPrintPreview *dlg) {
    printPreviewDialog = dlg;
    multiLineDelegate->setPreviewDialog(dlg);
}

void PropagatingTableView::updatePreview() {
    if (printPreviewDialog) {
        printPreviewDialog->updatePreview();
    }
}


void PropagatingTableView::showPrintPreview(bool showIt) {
    printPreviewDialog->setVisible(showIt);

    if (showIt) {
        printPreviewDialog->setWindowTitle(tr("Print Legends"));
        switch (printPreviewDialog->exec()) {
        case QDialog::Accepted:
            //setPageSetup();
            break;
        default:
            return;
        }
    }
}

void PropagatingTableView::setLabelFont(const QString &font) {
    if (codeWidget) {
        codeWidget->setLabelFont(font);
    }
    updatePreview();
}

void PropagatingTableView::setLabelSize_mm(const QSizeF &size_mm) {
    if (codeWidget) {
        codeWidget->setLabelSize_mm(size_mm);
    }
    updatePreview();
}

void PropagatingTableView::setFrameThickness(double thickness_mm) {
    if (codeWidget) {
        codeWidget->setFrameThickness(thickness_mm);
    }
    updatePreview();
}

QSizeF PropagatingTableView::labelSize_mm() {
    if (codeWidget) {
        return codeWidget->labelSize_mm();
    }
    return QSizeF();
}

QPageLayout PropagatingTableView::getPageLayout() {
    if (codeWidget) {
        return codeWidget->printer->pageLayout();
    }
    else {
        return QPageLayout();
    }
}

QRectF PropagatingTableView::getPageRect() {
    if (codeWidget) {
        return codeWidget->printer->pageRect(QPrinter::Millimeter);
    }
    else {
        return QRect();
    }
}


void PropagatingTableView::keyReleaseEvent(QKeyEvent *event) {
    //qDebug() << "PropagatingTableView::keyReleaseEvent";
    updatePreview();
    QTableView::keyReleaseEvent(event);
}


void PropagatingTableView::dragEnterEvent(QDragEnterEvent *event) {
    setBackgroundRole(QPalette::Highlight);

    event->acceptProposedAction();
    emit changed(event->mimeData());
}

void PropagatingTableView::dragMoveEvent(QDragMoveEvent *event) {
    event->acceptProposedAction();
}

void PropagatingTableView::dragLeaveEvent(QDragLeaveEvent *event) {
    event->accept();
}

void PropagatingTableView::dropEvent(QDropEvent *event) {
    const QMimeData *mimeData = event->mimeData();
    qDebug() << "PropagatingTableView::dropEvent" << mimeData->urls();
    static_cast<ImageLegendsModel*>(model())->appendUrls(mimeData->urls());
    event->acceptProposedAction();
    update();
}

void PropagatingTableView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) {
    QModelIndex index;
    foreach (index, selected.indexes()) {
        QModelIndex index = selected.indexes().constFirst();
        legendsModel.dataRows[index.row()].enterEditMode(index.column());
    }
    foreach (index, deselected.indexes()) {
        QModelIndex index = selected.indexes().constFirst();
        legendsModel.dataRows[index.row()].exitEditMode(index.column());
    }

    //model()->dataChanged(index, index, QList<int>() << Qt::ForegroundRole);
}



