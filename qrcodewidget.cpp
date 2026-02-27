#include <QPainter>
#include <QSvgRenderer>
#include <QIODevice>
#include <QKeyEvent>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

#include "cpp/qrcodegen.hpp"
#include "qrcodewidget.h"

using qrcodegen::QrCode;
using qrcodegen::QrSegment;
using std::atomic_uint8_t;



QrCodeWidget::QrCodeWidget(QWidget *parent)
    : Super{parent}
{
    if (NULL == printer) {
        printer = new QPrinter(QPrinter::HighResolution);
    }
}

QSizeF QrCodeWidget::paperSize_mm() const {
    //qDebug() << "QrCodeWidget::paperSize_mm" << printer->paperRect(QPrinter::Millimeter).size();
    return printer->paperRect(QPrinter::Millimeter).size();
}

QString QrCodeWidget::generateFileName(const QString &name) {
    QString  picturesLocation = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).value(0, QDir::currentPath());
    return picturesLocation + QLatin1String("/") + name;
}

void QrCodeWidget::setLandscapeOrientation(bool isLandscape) {
    QPageLayout::Orientation orientation = isLandscape ? QPageLayout::Landscape : QPageLayout::Portrait;
    printer->setPageOrientation(orientation);
    resizeContent();
}

void QrCodeWidget::setImageLegends(ImageLegendsModel *modelPtr) {
    svgDom.setImageLegends(modelPtr);
}

QSizeF QrCodeWidget::labelSize_mm() const {
    return QSizeF(svgDom.labelWidth_mm, svgDom.labelHeight_mm);
}

void QrCodeWidget::setLabelFont(const QString &font) {
    svgDom.labelFont = font;
}

void QrCodeWidget::setLabelSize_mm(const QSizeF &size_mm) {
    svgDom.labelWidth_mm = size_mm.width();
    svgDom.labelHeight_mm= size_mm.height();
}

void QrCodeWidget::setFrameThickness(double thickness) {
    svgDom.lineThickness_mm = thickness;
}

void QrCodeWidget::setPageNumber(int pageNumber) {
    _pageNumber = pageNumber;
    update();
}

int QrCodeWidget::pageNumber() const {
    return _pageNumber;
}

void QrCodeWidget::zoomIn(double factor) {
    displayFactor *= factor;
    resizeContent();
}

void QrCodeWidget::resetZoom() {
    displayFactor = defaultFactor;
    resizeContent();
}

int QrCodeWidget::pageCount() const {
    return 25;
}


QSize QrCodeWidget::sizeHint() const {
    return QSize(paperSize_mm().width()*displayFactor, paperSize_mm().height()*displayFactor);
}

void QrCodeWidget::resizeContent() {
    resize(sizeHint());
    update();
}

void QrCodeWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event)
    QPainter painter(this);
    painter.save();
    QSvgRenderer renderer(svgDom.onePageToSvg(_pageNumber-1, paperSize_mm()));
    renderer.setAspectRatioMode(Qt::KeepAspectRatio);
    renderer.render(&painter);
    painter.restore();

    // blue frame arround
    QPen pen(Qt::red, 4.0);
    painter.setPen(pen);
    painter.drawRect(rect());
}



void QrCodeWidget::print(QPrinter *printerPtr) {
    QPainter painter(printerPtr);

    qDebug() << "QrCodeWidget::print" << "/wd ht" << width() << height();

    double dpiX  = printerPtr->logicalDpiX();
    //double dpiY  = printerPtr->logicalDpiY();

    painter.save();
    double scale = dpiX * printerPtr->pageRect(QPrinter::Inch).width()/width();

    painter.scale(scale, scale);
    render(&painter);

    painter.restore();

    painter.end();
}


void QrCodeWidget::printPreview(QPrinter *printerPtr) {

    svgDom.printingFinished = false;
    QPainter painter;
    painter.begin(printerPtr);
    painter.save();
    painter.scale(1.0, 1.0);
    //const QSizeF pageSize_mm = printerPtr->pageRect(QPrinter::Millimeter).size();
    QSvgRenderer renderer(svgDom.onePageToSvg(0, paperSize_mm()));
    renderer.setAspectRatioMode(Qt::KeepAspectRatio);
    renderer.render(&painter);

    for (int page = 1; page < 100; ++page) {
        if (svgDom.printingFinished) break;
        printer->newPage();
        QSvgRenderer renderer(svgDom.onePageToSvg(page, paperSize_mm()));
        renderer.setAspectRatioMode(Qt::KeepAspectRatio);
        renderer.render(&painter);
    }
    painter.restore();
    painter.end();

}


void QrCodeWidget::finished(int value) {
    qDebug() << "QrCodeWidget::finished" << value;
}

QByteArray QrCodeWidget::svg(int pageNumberPlusOne) {
    return svgDom.onePageToSvg(pageNumberPlusOne-1, paperSize_mm());
}




