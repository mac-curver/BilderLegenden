#include <QPrintDialog>
#include "mainwindow.h"
#include "svgdisplay.h"
#include "ui_svgdisplay.h"

SvgDisplay::SvgDisplay(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::SvgDisplay)
{
    ui->setupUi(this);
}
}

SvgDisplay::~SvgDisplay() {
    delete ui;
}


void SvgDisplay::print(QPrinter *printerPtr) {
    ui->qrCodeWidget->print(printerPtr);
}

void SvgDisplay::setImageLegends(ImageLegendsModel *modelPtr) {
    ui->qrCodeWidget->setImageLegends(modelPtr);
}

void SvgDisplay::updateNumLines() {
    ui->qrCodeWidget->encodeQr(_pageNumber);
    ui->qrCodeWidget->resizeWithSizeHint();
}

void SvgDisplay::showEvent(QShowEvent *event) {
    Q_UNUSED(event)

    ui->qrCodeWidget->encodeQr(_pageNumber);
}

void SvgDisplay::closeEvent(QCloseEvent *event) {
    Q_UNUSED(event)
    static_cast<MainWindow*>(parent())->releaseShowButton();
}

void SvgDisplay::updateContents() {
    ui->qrCodeWidget->encodeQr(_pageNumber);
}

void SvgDisplay::setPageNumber(int page) {
    _pageNumber = page;
    ui->qrCodeWidget->encodeQr(_pageNumber);
}

void SvgDisplay::createPrinter() {
    qDebug() << "Print";
    QPrinter printer(QPrinter::HighResolution);
    printer.setPrinterName("desired printer name");
    QPrintDialog dialog(&printer, this);
    switch (dialog.exec()) {
    case QDialog::Rejected:
        return;
    default:
        print(&printer);
        return;
    }

}
