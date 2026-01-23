#ifndef QRCODEWIDGET_H
#define QRCODEWIDGET_H

#include <QPrinter>
#include <QSvgWidget>
#include <QAbstractTextDocumentLayout>

#include <cstdlib>
#include <cstring>

#include "cpp/qrcodegen.hpp"
#include "svgdom.h"


using std::atomic_uint8_t;
using qrcodegen::QrCode;
using qrcodegen::QrSegment;




class QrCodeWidget: public QWidget {
    Q_OBJECT
    typedef QWidget Super;
public:
    explicit QrCodeWidget(QWidget *parent = nullptr);

    QSizeF paperSize_mm() const;

    QString generateFileName(const QString &name);

    void setLandscapeOrientation(bool isLandscape);


    /// Returns a QByteArray of SVG code for an image depicting the given QR Code, with the given number
    /// of border modules. The QByteArray always uses Unix newlines (\n), regardless of the platform.

    void setImageLegends(ImageLegendsModel *modelPtr);

    void print(QPrinter *printerPtr);
    QSizeF labelSize_mm() const;
    void setLabelSize_mm(const QSizeF &size_mm);
    void setFrameThickness(double thickness);

    void setPageNumber(int pageNumber);
    int pageNumber() const;

    void zoomIn(double factor);
    void resetZoom();
    int pageCount() const;

public slots:
    void printPreview(QPrinter *printerPtr);
    void finished(int value);



protected:
    QSize sizeHint() const override;
    virtual void paintEvent(QPaintEvent *event) override;



public:
    SvgDom svgDom;
    int _pageNumber = 1;

private:
    const double defaultFactor = 3.0;

public:
    double displayFactor = defaultFactor;

    QPrinter *printer = NULL;


    void resizeContent();
};

#endif // QRCODEWIDGET_H
