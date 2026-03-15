#include <QPrintPreviewWidget>
#include <QPageLayout>
#include <QActionGroup>
#include "settings.h"
#include "customprintpreview.h"

/// As Qt6 removed object names from internal dialogs, we add them here for
/// the Portrait and Landscape toolbar buttons.
/// This is checked quite carefully but still might fail!
CustomPrintPreview::CustomPrintPreview(QPrinter *printer, QWidget *parent, Qt::WindowFlags flags):
    Super(printer, parent, flags)
{
    QPrintPreviewDialog *fakePreview = new QPrintPreviewDialog();
    QList<QAction *> allActions = fakePreview->findChildren<QAction*>();

    int portraitIndex = -1;
    int landscapeIndex = -1;
    for (int i = 0; i < allActions.count(); i++) {
        QAction *a = allActions[i];
        QActionGroup *g = a->actionGroup();
        if (a->isCheckable() && g && g->actions().size() == 2) {

            QPageLayout::Orientation before = fakePreview->printer()->pageLayout().orientation();
            // Simulate triggering without affecting UI
            a->trigger();

            QPageLayout::Orientation after = fakePreview->printer()->pageLayout().orientation();

            if (after != before) {
                // This action changes orientation
                if (after == QPageLayout::Portrait) {
                    portraitIndex = i;
                    landscapeIndex = i+1;
                }
                else {
                    portraitIndex = i-1;
                    landscapeIndex = i;
                }
            }
        }
    }
    delete fakePreview;
    if (portraitIndex >= 0 && landscapeIndex >= 0) {
        QList<QAction *> actions = findChildren<QAction*>();

        /// Re-invent the in Qt 6 canceled object names (might fail?!)
        actions[portraitIndex]->setObjectName("Portrait");
        actions[landscapeIndex]->setObjectName("Landscape");
    }
    //QPrintPreviewWidget* preview = findChild<QPrintPreviewWidget*>();
    //connect(preview, &QPrintPreviewWidget::previewChanged, this, &CustomPrintPreview::widgetPreviewChanged);
    //connect(preview, &QPrintPreviewWidget::paintRequested, this, &CustomPrintPreview::widgetPaintRequested);

}

CustomPrintPreview::~CustomPrintPreview() {

}



void CustomPrintPreview::accept() {
    // do abolutely nothing
    //setResult(QDialog::Rejected);
}

//void CustomPrintPreview::reject() {
//    // do abolutely nothing
//}

void CustomPrintPreview::updatePreview() {
    //QList<QPrintPreviewWidget*> list = findChildren<QPrintPreviewWidget*>();
    //QPrintPreviewWidget* preview = list.first();
    QPrintPreviewWidget* preview = findChild<QPrintPreviewWidget*>();
    if (preview) {
        preview->updatePreview();
    }
}



void CustomPrintPreview::storeSettings() {
    Settings::shared->storePageLayout(printer());
}



void CustomPrintPreview::retrieveSettings() {

    QPageLayout::Orientation orientation = printer()->pageLayout().orientation();

    QAction *portraitAction  = findChild<QAction *>("Portrait");
    QAction *landscapeAction = findChild<QAction *>("Landscape");


    if (portraitAction && landscapeAction) {
        portraitAction->setChecked(orientation == QPageLayout::Portrait);
        landscapeAction->setChecked(orientation == QPageLayout::Landscape);
    }

}

void CustomPrintPreview::closeEvent(QCloseEvent *) {
    QPageLayout layout = printer()->pageLayout();
    //qDebug() << "closeEvent" << printer()->pageLayout() << layout.orientation();
}

