#include <QPoint>
#include <QPainterPath>
#include <QTextStream>
#include <QtCore/qassert.h>

#include "settings.h"
#include "rowtype.h"
#include "svgdom.h"


SvgDom::SvgDom() {
}

void SvgDom::plotSingleLabel(const RowType &row, const QPointF &offset) {

    //QDomElement mask = makeMask("Mask", QPointF(), QSizeF(labelWidth_mm, labelHeight_mm));
    //root.appendChild(mask);

    QDomElement transform = createElement("g");
    transform.setAttribute("transform", QString("translate(%1 %2)").arg(offset.x()).arg(offset.y()));
    //transform.setAttribute("mask", "url(#Mask)"); // apply clipping

    QDomElement rectElement = makeRect(QPointF(), QSizeF(labelWidth_mm, labelHeight_mm));
    transform.appendChild(rectElement);

    QDomElement titleElement = makeCenteredText(row.title, titleSize, titleYPosition);
    transform.appendChild(titleElement);

    double vertical = descriptionYPosition;
    if (Settings::shared->addPhotoInfo()) {
        QDomElement lineElement = makeCenteredText(row.camera, infoSize, vertical-infoSize);
        transform.appendChild(lineElement);
        vertical += infoSize*lineHeightFactor;
    }
    for (int i = 0; i < row.descriptions.count(); i++) {
        QDomElement lineElement = makeCenteredText(row.descriptions.at(i), descriptionSize, vertical);
        vertical += descriptionSize*lineHeightFactor;
        transform.appendChild(lineElement);
    }
    QDomElement authorElement = makeCenteredText(row.author, authorSize, labelHeight_mm-border_mm);
    transform.appendChild(authorElement);

    const QrCode qr = QrCode::encodeText(row.url.toStdString().c_str(), QrCode::Ecc::MEDIUM);

    if (!row.url.isEmpty()) {
        const QrCode qr = QrCode::encodeText(row.url.toStdString().c_str(), QrCode::Ecc::MEDIUM);
        QDomElement translateTransform = createElement("g");
        double qrXPosition = labelWidth_mm-matrixCodeScale*qr.getSize()-border_mm;
        switch (Settings::shared->labelAlignment()) {
        case Qt::AlignRight:
            qrXPosition = border_mm;
        }

        QPointF translation(qrXPosition, labelHeight_mm-matrixCodeScale*qr.getSize()-border_mm);
        translateTransform.setAttribute("transform", QString("translate(%1 %2)").arg(translation.x()).arg(translation.y()));
        QDomElement scaleTransform = createElement("g");
        scaleTransform.setAttribute("transform", QString("scale(%1 %2)").arg(matrixCodeScale).arg(matrixCodeScale));
        scaleTransform.appendChild(makeQrCode(qr));
        translateTransform.appendChild(scaleTransform);
        transform.appendChild(translateTransform);
    }

    root.appendChild(transform);
}

QSizeF SvgDom::plotSize(const QSizeF &pageSize_mm) const {
    return QSizeF(2*pageMargin +  number_width(pageSize_mm)*labelWidth_mm,
                  2*pageMargin + number_height(pageSize_mm)*labelHeight_mm
           );
}

int SvgDom::number_width(const QSizeF &pageSize_mm) const {
    //qDebug() << "SvgDom::number_width" << pageSize_mm.width() << labelWidth_mm << (pageSize_mm.width() - 2*pageMargin) / labelWidth_mm;
    return (pageSize_mm.width() - 2*pageMargin) / labelWidth_mm;
}

int SvgDom::number_height(const QSizeF &pageSize_mm) const {
    return (pageSize_mm.height() - 2*pageMargin) / labelHeight_mm;;
}




QByteArray SvgDom::onePageToSvg(int page, const QSizeF &pageSize_mm) {
    clear();

    QSizeF size_mm = plotSize(pageSize_mm);

    root = createElement("svg");
    root.setAttribute("viewBox", QString("0 0 %1 %2")
                                     .arg(size_mm.width())
                                     .arg(size_mm.height())
    );
    root.setAttribute("stroke", "none");
    root.setAttribute("width",  QString("%1").arg(size_mm.width()));
    root.setAttribute("height", QString("%1").arg(size_mm.height()));

    root.setAttribute("overflow", "visible");
    root.setAttribute("xmlns", "http://www.w3.org/2000/svg");
    appendChild(root);

    int labelsPerHeight = number_height(pageSize_mm);
    int labelsPerWidth = number_width(pageSize_mm);
    int firstElement_page = page*labelsPerWidth*labelsPerHeight;
    int elementNumber = firstElement_page;
    for (int v = 0; v < labelsPerHeight; v++) {
        for (int h = 0; h < labelsPerWidth; h++) {
            QPointF pageOffset = QPointF(pageMargin + h * labelWidth_mm, pageMargin + v * labelHeight_mm);
            if (elementNumber < modelPtr->rowCount()) {
                RowType row = modelPtr->getRow(elementNumber);
                if (!row.title.isEmpty()) {
                    plotSingleLabel(row, pageOffset);
                }
            }
            else {
                goto finishPage;
            }
            elementNumber++;
        }
    }
finishPage:

    if (Settings::shared->addCuttingLine()) {
        QDomElement grid = makeCuttingGrid(pageSize_mm, "black", 0.02);
        root.appendChild(grid);
    }


    if (elementNumber >= modelPtr->rowCount()) {
        printingFinished = true;
    }
    return toString().toUtf8();
}

QDomElement SvgDom::createPrimitiveRect(const QPointF &offset, const QSizeF &size, double thickness) {
    QDomElement rect = createElement("rect");
    rect.setAttribute("x", QString("%1").arg(offset.x()+thickness*0.5));
    rect.setAttribute("y", QString("%1").arg(offset.y()+thickness*0.5));
    rect.setAttribute("width", QString("%1").arg(size.width()-thickness));
    rect.setAttribute("height", QString("%1").arg(size.height()-thickness));
    return rect;
}


QDomElement SvgDom::makeClip(const QString &id, const QPointF &offset, const QSizeF &size) {
    Q_ASSERT_X(false, "Qt does not support clip-path", "svgdom.cpp");
    QDomElement defs = createElement("defs");
    QDomElement clip = createElement("clipPath");
    clip.setAttribute("id", id);
    QDomElement rect = createPrimitiveRect(offset, size, 0);
    clip.appendChild(rect);
    defs.appendChild(clip);
    return defs;
}



QDomElement SvgDom::makeMask(const QString &id, const QPointF &offset, const QSizeF &size) {
    Q_ASSERT_X(false, "Qt mask looks ugly", "svgdom.cpp");
    QDomElement defs = createElement("defs");
    QDomElement mask = createElement("mask");
    mask.setAttribute("id", id);
    QDomElement rect = createPrimitiveRect(offset, size, 0);
    rect.setAttribute("fill", "white");
    mask.appendChild(rect);
    defs.appendChild(mask);
    return defs;

}


QDomElement SvgDom::makeRect(const QPointF &offset, const QSizeF &size) {
    QDomElement rect = createPrimitiveRect(offset, size, lineThickness_mm);
    if (Settings::shared->whiteOnBlack()) {
        rect.setAttribute("fill", "#000000");
    }
    else {
        rect.setAttribute("fill", "#ffffff");
    }
    rect.setAttribute("stroke", "black");
    rect.setAttribute("stroke-width", lineThickness_mm);
    return rect;
}

QDomElement SvgDom::makeCenteredText(const QString &text, int size_px, int yPosition) {
    if (Settings::shared->preserveCharacterSpacing()) {
        return makeCenteredTextAsPath(text, size_px, yPosition);
    }
    else {
        return makeCenteredTextAsText(text, size_px, yPosition);
    }
}

QDomElement SvgDom::makeCenteredTextAsText(
    const QString &text,
    int size_px,
    int yPosition
) {
    QDomElement title = createElement("text");

    title.setAttribute("font-size", QString("%1px").arg(size_px));
    title.setAttribute("font-family", QString("%2").arg(labelFont));

    switch (Settings::shared->labelAlignment()) {
    case Qt::AlignRight:
        title.setAttribute("text-anchor", "end");
        title.setAttribute("x", labelWidth_mm-border_mm);
        break;
    case Qt::AlignLeft:
        title.setAttribute("text-anchor", "start");
        title.setAttribute("x", border_mm);
        break;
    default:
        title.setAttribute("text-anchor", "middle");
        title.setAttribute("x", labelWidth_mm/2.0);
    }

    title.setAttribute("y", yPosition);
    if (Settings::shared->whiteOnBlack()) {
        title.setAttribute("fill", "white");
    }
    else {
        title.setAttribute("fill", "black");
    }
    title.appendChild(createTextNode(text));
    return title;

}

/// KI Generated code
QDomElement SvgDom::makeCenteredTextAsPath(
    const QString &text,
    int size_px,
    int yPosition
) {
    // 1. Font vorbereiten
    QFont font(labelFont);
    font.setPixelSize(size_px);

    // 2. Pfad erzeugen
    QPainterPath path;
    path.addText(0, 0, font, text);

    // 3. Bounding Box bestimmen
    QRectF bounds = path.boundingRect();

    // 4. X‑Position zentrieren
    double xOffset = (labelWidth_mm-bounds.width()) / 2.0;

    switch (Settings::shared->labelAlignment()) {
    case Qt::AlignRight:
        xOffset = labelWidth_mm - bounds.width() - border_mm;
        break;
    case Qt::AlignLeft:
        xOffset = border_mm;
        break;
    default:
        break;
    }


    // 5. SVG‑Pfadstring erzeugen
    QString svgPath;

    if (!path.isEmpty()) {
        QPainterPath::Element e = path.elementAt(0);

        double x = e.x + xOffset;
        double y = e.y + yPosition;
        svgPath += QString("M %1 %2 ").arg(x).arg(y);

        for (int i = 1; i < path.elementCount(); ++i) {
            e = path.elementAt(i);

            x = e.x + xOffset;
            y = e.y + yPosition;
            if (e.isMoveTo()) {
                svgPath += QString("M %1 %2 ").arg(x).arg(y);
            }
            else if (e.isLineTo()) {
                svgPath += QString("L %1 %2 ").arg(x).arg(y);
            }
            else if (e.isCurveTo()) {
                double x1 = path.elementAt(i+1).x + xOffset;
                double y1 = path.elementAt(i+1).y + yPosition;
                double x2 = path.elementAt(i+2).x + xOffset;
                double y2 = path.elementAt(i+2).y + yPosition;

                svgPath += QString("C %1 %2 %3 %4 %5 %6 ")
                               .arg(x).arg(y)
                               .arg(x1).arg(y1)
                               .arg(x2).arg(y2)
                    ;
            }
        }
        svgPath += QString("Z");
    }
    // 6. SVG‑Element erzeugen
    QDomElement pathElem = createElement("path");
    pathElem.setAttribute("d", svgPath);
    if (Settings::shared->whiteOnBlack()) {
        pathElem.setAttribute("fill", "white");
        pathElem.setAttribute("stroke", "white");
    }
    else {
        pathElem.setAttribute("fill", "black");
        pathElem.setAttribute("stroke", "black");
    }
    pathElem.setAttribute("stroke-width", "0.1");

    return pathElem;
}


QDomElement SvgDom::makeQrCode(const QrCode &qr) {

    QDomElement qrElement = createElement("path");
    if (Settings::shared->whiteOnBlack()) {
        qrElement.setAttribute("fill", "white");
    }
    else {
        qrElement.setAttribute("fill", "black");
    }
    qrElement.setAttribute("stroke-width", "1");
    QString matrix;
    QTextStream stream(&matrix);
    for (int y = 0; y < qr.getSize(); y++) {
        for (int x = 0; x < qr.getSize(); x++) {
            if (qr.getModule(x, y)) {
                if (x != 0 || y != 0) {
                    stream << " ";
                }
                stream << "M" << x << " " << y << "h1v1h-1z";
            }
        }
    }

    qrElement.setAttribute("d", matrix);
    return qrElement;
}

QDomElement SvgDom::makeCuttingGrid(const QSizeF &pageSize_mm, const QString &color, double lineWidth) {

    QDomElement grid = createElement("g");
    int labelsPerHeight = number_height(pageSize_mm);
    int labelsPerWidth = number_width(pageSize_mm);

    for (int v = 0; v < labelsPerHeight+1; v++) {
        QDomElement line = createElement("line");
        line.setAttribute("x1", QString("%1").arg(-90));
        line.setAttribute("y1", QString("%1").arg(pageMargin+v*labelHeight_mm));
        line.setAttribute("x2", QString("%1").arg(pageSize_mm.width()));
        line.setAttribute("y2", QString("%1").arg(pageMargin+v*labelHeight_mm));
        line.setAttribute("stroke", color);
        line.setAttribute("stroke-width", lineWidth);
        grid.appendChild(line);
    }

    for (int h = 0; h < labelsPerWidth+1; h++) {
        QDomElement line = createElement("line");
        line.setAttribute("x1", QString("%1").arg(pageMargin+h*labelWidth_mm));
        line.setAttribute("y1", QString("%1").arg(-20));
        line.setAttribute("x2", QString("%1").arg(pageMargin+h*labelWidth_mm));
        line.setAttribute("y2", QString("%1").arg(pageSize_mm.height()));
        line.setAttribute("stroke", color);
        line.setAttribute("stroke-width", lineWidth);
        grid.appendChild(line);
    }
    return grid;
}


/*
void SvgDom::addRect() {
    root.appendChild(makeRect(QPointF(), QSizeF(labelWidth_mm, labelHeight_mm)));
}

void SvgDom::addText(const QString &text, double size_px, double yPosition) {
    root.appendChild(makeCenteredTextAsPath(text, size_px, yPosition));
}
*/



void SvgDom::setImageLegends(ImageLegendsModel *modelPtr) {
    this->modelPtr = modelPtr;
}

int SvgDom::numberOfRows() const {
    return modelPtr ? modelPtr->rowCount() : 1;
}
