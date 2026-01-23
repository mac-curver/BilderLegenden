#include <QPrintPreviewWidget>
#include "customprintpreview.h"


CustomPrintPreview::CustomPrintPreview(QPrinter *printer, QWidget *parent, Qt::WindowFlags flags):
    Super(printer, parent, flags)
{

}

CustomPrintPreview::~CustomPrintPreview() {

}


void CustomPrintPreview::accept() {
    // do abolutely nothings
    //setResult(QDialog::Rejected);
}

//void CustomPrintPreview::reject() {
//    // do abolutely nothings
//}

void CustomPrintPreview::updatePreview() {
    QList<QPrintPreviewWidget*> list = findChildren<QPrintPreviewWidget*>();
    QPrintPreviewWidget* preview = list.first();
    preview->updatePreview();
}
