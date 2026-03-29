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


    QDomElement transform = createElement("g");
    transform.setAttribute("transform", QString("translate(%1 %2)").arg(offset.x()).arg(offset.y()));

    QDomElement rectElement = makeOuterRect(QPointF(), QSizeF(labelWidth_mm, labelHeight_mm));
    transform.appendChild(rectElement);

    QDomElement innerElement = makeInnerRect(QPointF(), QSizeF(labelWidth_mm, labelHeight_mm));
    transform.appendChild(innerElement);


    QDomElement titleElement = makeCenteredText(row.title, titleSize, titleYPosition + outerThickness_mm);
    transform.appendChild(titleElement);

    double vertical = descriptionYPosition + outerThickness_mm;
    if (Settings::shared->addPhotoInfo()) {
        QDomElement lineElement = makeCenteredText(row.camera, infoSize, vertical-infoSize);
        transform.appendChild(lineElement);
        vertical += infoSize*lineHeightFactor;
    }
    for (int i = 0; i < row.descriptions.count(); i++) {
        if (!row.descriptions.at(i).isEmpty()) {
            QDomElement lineElement = makeCenteredText(row.descriptions.at(i), descriptionSize, vertical);
            vertical += descriptionSize*lineHeightFactor;
            transform.appendChild(lineElement);
        }
    }
    double lowerBorder = border_mm + outerThickness_mm;
    QDomElement authorElement = makeCenteredText(row.author, authorSize, labelHeight_mm-lowerBorder);
    transform.appendChild(authorElement);

    const QrCode qr = QrCode::encodeText(row.url.toStdString().c_str(), QrCode::Ecc::MEDIUM);

    if (!row.url.isEmpty()) {
        const QrCode qr = QrCode::encodeText(row.url.toStdString().c_str(), QrCode::Ecc::MEDIUM);
        QDomElement translateTransform = createElement("g");
        double qrXPosition = labelWidth_mm-matrixCodeScale*qr.getSize()-lowerBorder;
        switch (Settings::shared->labelAlignment()) {
        case Qt::AlignRight:
            qrXPosition = lowerBorder;
            break;
        default:
            break;
        }

        QPointF translation(qrXPosition, labelHeight_mm-matrixCodeScale*qr.getSize()-lowerBorder);
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
    return QSizeF(2*pageMargin + number_width(pageSize_mm) *labelWidth_mm,
                  2*pageMargin + number_height(pageSize_mm)*labelHeight_mm
           );
}

int SvgDom::number_width(const QSizeF &pageSize_mm) const {
    return (pageSize_mm.width() - 2*pageMargin) / labelWidth_mm;
}

int SvgDom::number_height(const QSizeF &pageSize_mm) const {
    return (pageSize_mm.height() - 2*pageMargin) / labelHeight_mm;;
}




QByteArray SvgDom::onePageToSvg(int page, const QSizeF &pageSize_mm) {
    clear();

    root = createElement("svg");

    /*
    QSizeF size_mm = plotSize(pageSize_mm);
    // Must be done for the renderer, otherwise strange scalings.
    root.setAttribute("width",  QString("%1").arg(size_mm.width()));
    root.setAttribute("height", QString("%1").arg(size_mm.height()));
    root.setAttribute("viewBox", QString("0 0 %1 %2")
                                     .arg(size_mm.width())
                                     .arg(size_mm.height())
    );
    */
    root.setAttribute("stroke", "none");
    //root.setAttribute("overflow", "visible");
    root.setAttribute("xmlns", "http://www.w3.org/2000/svg");
    appendChild(root);

    int labelsPerHeight = number_height(pageSize_mm);
    int labelsPerWidth = number_width(pageSize_mm);
    int firstElement_page = page*labelsPerWidth*labelsPerHeight;
    int elementNumber = firstElement_page;

    for (int v = 0; v < labelsPerHeight; v++) {
        for (int h = 0; h < labelsPerWidth; h++) {
            QPointF pageOffset = QPointF(pageMargin + h * labelWidth_mm,
                                         pageMargin + v * labelHeight_mm
                    );
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
    /*
    QDomElement pageRect = createPrimitiveRect(QPointF(0, 0), QSizeF(pageSize_mm.width()*0.5, pageSize_mm.height()*0.5), 1.0);

    pageRect.setAttribute("stroke", "black");
    pageRect.setAttribute("stroke-width", 1.0);
    pageRect.setAttribute("fill", "#ffffff");
    root.appendChild(pageRect);

    for (int v = 0; v < labelsPerHeight; v++) {
        for (int h = 0; h < labelsPerWidth; h++) {
            QPointF pageOffset = QPointF(pageMargin + h * labelWidth_mm, pageMargin + v * labelHeight_mm);
            QDomElement transform = createElement("g");
            transform.setAttribute("transform", QString("translate(%1 %2)").arg(pageOffset.x()).arg(pageOffset.y()));
            QDomElement text = makeCenteredText(QString("%1 x %2").arg(labelWidth_mm).arg(labelHeight_mm), 10, 10);
            transform.appendChild(text);
            root.appendChild(transform);
        }
    }
    */
    if (Settings::shared->addCuttingLine()) {
        QDomElement grid = makeCuttingGrid(pageSize_mm, "black", 0.1);
        root.appendChild(grid);
    }


    if (elementNumber >= modelPtr->rowCount()) {
        printingFinished = true;
    }
    //printf("%s", toString().toStdString().c_str());
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


QDomElement SvgDom::makeOuterRect(const QPointF &offset, const QSizeF &size) {
    QDomElement outerRect = createPrimitiveRect(offset, size, outerThickness_mm);
    if (Settings::shared->whiteOnBlack()) {
        outerRect.setAttribute("stroke", "black");
    }
    else {
        outerRect.setAttribute("stroke", "white");

    }
    outerRect.setAttribute("fill", "none");
    outerRect.setAttribute("stroke-width", outerThickness_mm);
    return outerRect;
}

QDomElement SvgDom::makeInnerRect(const QPointF &offset, const QSizeF &size) {
    QDomElement innerRect = createPrimitiveRect(offset, size, 2*(lineThickness_mm+outerThickness_mm));
    if (Settings::shared->whiteOnBlack()) {
        innerRect.setAttribute("stroke", "white");
        innerRect.setAttribute("fill", "black");
    }
    else {
        innerRect.setAttribute("stroke", "black");
        innerRect.setAttribute("fill", "none");
    }
    innerRect.setAttribute("stroke-width", lineThickness_mm);

    return innerRect;
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
        line.setAttribute("x1", QString("%1").arg(-5));
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
        line.setAttribute("y1", QString("%1").arg(-labelWidth_mm/2));
        line.setAttribute("x2", QString("%1").arg(pageMargin+h*labelWidth_mm));
        line.setAttribute("y2", QString("%1").arg(pageSize_mm.height()));
        line.setAttribute("stroke", color);
        line.setAttribute("stroke-width", lineWidth);
        grid.appendChild(line);
    }
    return grid;
}



void SvgDom::setImageLegends(ImageLegendsModel *modelPtr) {
    this->modelPtr = modelPtr;
}

int SvgDom::numberOfRows() const {
    return modelPtr ? modelPtr->rowCount() : 1;
}
