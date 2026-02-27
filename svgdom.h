#ifndef SVGDOM_H
#define SVGDOM_H

#include <QObject>
#include <QDomDocument>

#include "imagelegendsmodel.h"
#include "cpp/qrcodegen.hpp"
using qrcodegen::QrCode;


class SvgDom: public QDomDocument {
    typedef QDomDocument Super;

public:
    SvgDom();

    QByteArray onePageToSvg(int page, const QSizeF &pageSize_mm);

    QDomElement makeClip(const QString &id, const QPointF &offset, const QSizeF &size); // can't be used
    QDomElement makeMask(const QString &id, const QPointF &offset, const QSizeF &size); // can't be used
    QDomElement makeRect(const QPointF &offset, const QSizeF &size);
    QDomElement makeCenteredText(const QString &text, int size_px, int yPosition);
    QDomElement makeQrCode(const QrCode &qr);


    //void addRect();
    //void addText(const QString &text, double size_px, double yPosition);

    void setViewBox(double width, double height);

    void setImageLegends(ImageLegendsModel *modelPtr);
    int numberOfRows() const;
    void plotSingleLabel(int line, const QPointF &offset);
    bool printingFinished = false;


    QSizeF plotSize(const QSizeF &pageSize_mm) const;
private:
    QDomElement makeCenteredTextAsText(const QString &text, int size_px, int yPosition);
    QDomElement makeCenteredTextAsPath(const QString &text, int size_px, int yPosition);


public:
    int number_width(const QSizeF &pageSize_mm) const;
    int number_height(const QSizeF &pageSize_mm) const;

public:
    double pageMargin = 1.05;
    double labelWidth_mm =  100.0;
    double labelHeight_mm =  50.0;
    double lineThickness_mm = 1.0;
    QString labelFont = "Arial";

private:
    double border = 7.0;
    double titleYPosition = 16.0;
    double titleSize = 9.0;
    double descriptionYPosition = titleYPosition + 8.0;
    double descriptionSize = 5.0;
    double authorSize = 6.0;
    double lineSpacingY = descriptionSize*1.2;
    double matrixCodeScale = 0.45;


private:
    QDomElement root;
    ImageLegendsModel *modelPtr;

};

#endif // SVGDOM_H
