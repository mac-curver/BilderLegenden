#include <QPrintPreviewWidget>
#include <QPageLayout>
#include <QComboBox>
#include "settings.h"
#include "customprintpreview.h"

/// As Qt6 removed object names from internal dialogs, we add them here for
/// the Portrait and Landscape toolbar buttons.
/// This is checked quite carefully but still might fail!
CustomPrintPreview::CustomPrintPreview(QPrinter *printer, QWidget *parent, Qt::WindowFlags flags):
    Super(printer, parent, flags)
{
    /*
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
    */
    preview = findChild<QPrintPreviewWidget*>();
    /// Enable zoom on MAC
    QList<QComboBox *> items = findChildren<QComboBox*>();
    connect(items.front(), &QComboBox::currentTextChanged, this, &CustomPrintPreview::zoom);

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


/// Added as at least on MAC zoom is not working
/// zoom the contained widget according to the selected text
/// The text is like: "45,5%"
void CustomPrintPreview::zoom(const QString &zoomAsText) {
    QString doubleAsText = zoomAsText;
    doubleAsText.chop(1);                                              // delete the % sign
    bool ok;
    // convert to percent
    double zoomPercent = doubleAsText.replace(",", ".").toDouble(&ok)/100;
    if (ok) {
        if (preview) {
            preview->setZoomFactor(zoomPercent);
        }
    }

}

