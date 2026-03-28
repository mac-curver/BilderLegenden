#ifndef PROPAGATINGTABLEVIEW_H
#define PROPAGATINGTABLEVIEW_H

#include <QTableView>
#include "customprintpreview.h"
#include "qrcodewidget.h"
#include "centereddelegate.h"
#include "multilinedelegate.h"

class PropagatingTableView : public QTableView {
    Q_OBJECT
    typedef QTableView Super;
public:
    explicit PropagatingTableView(QWidget *parent = nullptr);
    ~PropagatingTableView();
    void setColumnWidths();
    void openExifFile(const QString &fileName);
    void setPreviewDialog(CustomPrintPreview *dlg);

    void updatePreview();
    void showPrintPreview(bool showIt);
    void setLabelFont(const QString &font);
    void setLabelSize_mm(const QSizeF &size_mm);
    void setFrameThickness(double thickness_mm);
    void setOuterThickness(double thickness_mm);

    void storePrintSettings();
    void retrievePrintSettings();



    QSizeF labelSize_mm();

    QPageLayout getPageLayout();
    QRectF getPageRect();

    void retrieveColumnWidths();
    void storeColumnWidths();


protected:
    virtual void keyReleaseEvent(QKeyEvent *event) override;

    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;

    void dropEvent(QDropEvent *event) override;
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override;



signals:
    void changed(const QMimeData *mimeData = nullptr);
    void requestUpdate();



private:
    ImageLegendsModel legendsModel;
    MultiLineDelegate *multiLineDelegate = NULL;
    CenteredDelegate *centerDelegate = NULL;
    CustomPrintPreview *printPreviewDialog = NULL;

    QMap<RowType::ColumnType, int> columnWidthsMap;
    QMap<RowType::ColumnType, int> defaultColumnWidthsMap;



public:
    QrCodeWidget *codeWidget = NULL;


};

#endif // PROPAGATINGTABLEVIEW_H
