#include <QDebug>
#include <QMimeData>
#include <QMessageBox>
#include <QPrintPreviewWidget>
#include <QFile>
#include "settings.h"
#include "customprintpreview.h"
#include "multilinedelegate.h"


#include "propagatingtableview.h"

PropagatingTableView::PropagatingTableView(QWidget *parent): QTableView(parent) {
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
    Settings::shared->retrievePageLayout(codeWidget->printer);

    printPreviewDialog = new CustomPrintPreview(codeWidget->printer, this);

    codeWidget->setImageLegends(static_cast<ImageLegendsModel*>(this->model()));
    connect(printPreviewDialog, &CustomPrintPreview::paintRequested, codeWidget, &QrCodeWidget::printPreview);

    defaultColumnWidthsMap = {
        {RowType::Image, 70},
        {RowType::Title, 150},
        {RowType::Description, 250},
        {RowType::Camera, 120},
        {RowType::Author, 150},
        {RowType::Url_QR, 0200}
    };

    columnWidthsMap = QMap(defaultColumnWidthsMap);

    retrieveColumnWidths();
    resizeRowsToContents();
}

PropagatingTableView::~PropagatingTableView() {
    delete multiLineDelegate;
}

void PropagatingTableView::setColumnWidths() {

    for (int column = 0; column < RowType::N_ColumnTypes-1; column++) {
        RowType::ColumnType type = static_cast<RowType::ColumnType>(column);
        setColumnWidth(column, columnWidthsMap[type]);
    }
}


void PropagatingTableView::updatePreview() {
    if (codeWidget && printPreviewDialog) {
        codeWidget->isPrinter = false;
        printPreviewDialog->updatePreview();
        codeWidget->isPrinter = true;
    }
}


void PropagatingTableView::showPrintPreview(bool showIt) {
    //QPageLayout layoutWithLandScape;
    //layoutWithLandScape.setOrientation(QPageLayout::Landscape);
    //codeWidget->printer->setPageLayout(layoutWithLandScape);

    if (showIt) {
        codeWidget->isPrinter = false;
        printPreviewDialog->setVisible(true);
        printPreviewDialog->raise();
        printPreviewDialog->setWindowTitle(tr("Print Legends"));
        switch (printPreviewDialog->exec()) {
        case QDialog::Accepted:
            //setPageSetup();
            break;
        default:
            return;
        }
        codeWidget->isPrinter = true;
    }
    else {
        printPreviewDialog->setVisible(false);
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


void PropagatingTableView::storePrintSettings() {
    if (printPreviewDialog) {
        printPreviewDialog->storeSettings();
    }
}

void PropagatingTableView::retrievePrintSettings() {
    if (printPreviewDialog) {
        printPreviewDialog->retrieveSettings();
    }
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

void PropagatingTableView::retrieveColumnWidths() {
    Settings::shared->beginGroup("Table");
    Settings::shared->beginGroup("Widths");
    for (int i = 0; i < RowType::N_ColumnTypes; i++) {
        RowType::ColumnType type = static_cast<RowType::ColumnType>(i);
        QString typeAsString = QVariant::fromValue(type).toString();
        qDebug() << typeAsString;
        columnWidthsMap[type] = Settings::shared->value(typeAsString, columnWidthsMap[type]).toInt();
    }

    Settings::shared->endGroup();
    Settings::shared->endGroup();
}

void PropagatingTableView::storeColumnWidths() {
    Settings::shared->beginGroup("Table");
    Settings::shared->beginGroup("Widths");
    for (int i = 0; i < RowType::N_ColumnTypes; i++) {
        RowType::ColumnType type = static_cast<RowType::ColumnType>(i);
        QString typeAsString = QVariant::fromValue(type).toString();
        Settings::shared->setValue(typeAsString, columnWidth(i));
    }
    Settings::shared->endGroup();
    Settings::shared->endGroup();
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
    static_cast<ImageLegendsModel*>(model())->appendUrls(mimeData->urls());
    event->acceptProposedAction();
    update();
}

void PropagatingTableView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) {
    QModelIndex index;
    foreach (index, selected.indexes()) {
        //QModelIndex index = selected.indexes().constFirst();
        legendsModel.dataRows[index.row()].enterEditMode(index.column());
    }
    foreach (index, deselected.indexes()) {
        //QModelIndex index = selected.indexes().constFirst();
        legendsModel.dataRows[index.row()].exitEditMode(index.column());
    }

    //model()->dataChanged(index, index, QList<int>() << Qt::ForegroundRole);
}



