#include <QPoint>
#include <QPainterPath>
#include <QTextStream>
#include <QtCore/qassert.h>

#include "settings.h"
#include "rowtype.h"
#include "svgdom.h"


SvgDom::SvgDom() {
}

void SvgDom::plotSingleLabel(int line, const QPointF &offset) {

    //QDomElement mask = makeMask("Mask", QPointF(), QSizeF(labelWidth_mm, labelHeight_mm));
    //root.appendChild(mask);

    QDomElement transform = createElement("g");
    transform.setAttribute("transform", QString("translate(%1 %2)").arg(offset.x()).arg(offset.y()));
    //transform.setAttribute("mask", "url(#Mask)"); // apply clipping

    QDomElement rectElement = makeRect(QPointF(), QSizeF(labelWidth_mm, labelHeight_mm));
    transform.appendChild(rectElement);

    RowType row = modelPtr->getRow(line);
    QDomElement titleElement = makeCenteredText(row.title, titleSize, titleYPosition);
    transform.appendChild(titleElement);
    for (int i = 0; i < row.descriptions.count(); i++) {
        QDomElement lineElement = makeCenteredText(row.descriptions.at(i), descriptionSize, descriptionYPosition+i*lineSpacingY);
        transform.appendChild(lineElement);
    }
    QDomElement authorElement = makeCenteredText(row.author, authorSize, labelHeight_mm-border);
    transform.appendChild(authorElement);

    const QrCode qr = QrCode::encodeText(row.url.toStdString().c_str(), QrCode::Ecc::MEDIUM);

    if (!row.url.isEmpty()) {
        const QrCode qr = QrCode::encodeText(row.url.toStdString().c_str(), QrCode::Ecc::MEDIUM);
        QDomElement translateTransform = createElement("g");
        QPointF translation(labelWidth_mm-matrixCodeScale*qr.getSize()-border, labelHeight_mm-matrixCodeScale*qr.getSize()-border);
        translateTransform.setAttribute("transform", QString("translate(%1 %2)").arg(translation.x()).arg(translation.y()));
        QDomElement scaleTransform = createElement("g");
        scaleTransform.setAttribute("transform", QString("scale(%1 %2)").arg(matrixCodeScale).arg(matrixCodeScale));
        scaleTransform.appendChild(makeQrCode(qr));
        translateTransform.appendChild(scaleTransform);
        transform.appendChild(translateTransform);
    }

    root.appendChild(transform);
    //qDebug() << toString();
}

QSizeF SvgDom::plotSize(const QSizeF &pageSize_mm) const {
    return QSizeF(2*pageMargin + number_width(pageSize_mm)*labelWidth_mm,
                 2*pageMargin + number_height(pageSize_mm)*labelHeight_mm
           );
}

int SvgDom::number_width(const QSizeF &pageSize_mm) const {
    qDebug() << "SvgDom::number_width" << pageSize_mm.width() << labelWidth_mm << (pageSize_mm.width() - 2*pageMargin) / labelWidth_mm;
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

    //root.setAttribute("overflow", "hidden");
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
                plotSingleLabel(elementNumber, pageOffset);
            }
            else {
                goto finishPage;
            }
            elementNumber++;
        }
    }
finishPage:
    if (elementNumber >= modelPtr->rowCount()) {
        printingFinished = true;
    }
    return toString().toUtf8();
}


QDomElement SvgDom::makeClip(const QString &id, const QPointF &offset, const QSizeF &size) {
    Q_ASSERT_X(false, "Qt does not support clip-path", "svgdom.cpp");
    QDomElement defs = createElement("defs");
    QDomElement clip = createElement("clipPath");
    clip.setAttribute("id", id);
    QDomElement rect = createElement("rect");
    rect.setAttribute("x", QString("%1").arg(offset.x()));
    rect.setAttribute("y", QString("%1").arg(offset.y()));
    rect.setAttribute("width", QString("%1").arg(size.width()));
    rect.setAttribute("height", QString("%1").arg(size.height()));
    clip.appendChild(rect);
    defs.appendChild(clip);
    return defs;
}

QDomElement SvgDom::makeMask(const QString &id, const QPointF &offset, const QSizeF &size) {
    Q_ASSERT_X(false, "Qt mask looks ugly", "svgdom.cpp");
    QDomElement defs = createElement("defs");
    QDomElement mask = createElement("mask");
    mask.setAttribute("id", id);
    QDomElement rect = createElement("rect");
    rect.setAttribute("x", QString("%1").arg(offset.x()));
    rect.setAttribute("y", QString("%1").arg(offset.y()));
    rect.setAttribute("width", QString("%1").arg(size.width()));
    rect.setAttribute("height", QString("%1").arg(size.height()));
    rect.setAttribute("fill", "white");
    mask.appendChild(rect);
    defs.appendChild(mask);
    return defs;

}



QDomElement SvgDom::makeRect(const QPointF &offset, const QSizeF &size) {
    QDomElement rect = createElement("rect");
    rect.setAttribute("x", QString("%1").arg(offset.x()));
    rect.setAttribute("y", QString("%1").arg(offset.y()));
    rect.setAttribute("width", QString("%1").arg(size.width()));
    rect.setAttribute("height", QString("%1").arg(size.height()));
    rect.setAttribute("fill", "#ffffff");
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
    title.setAttribute("style", QString("font-size:%1px;font-family:%2;text-align:center;text-anchor:middle")
                                    .arg(size_px).arg(labelFont));
    title.setAttribute("x", labelWidth_mm/2.0);
    title.setAttribute("y", yPosition);
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
    double xCenter = labelWidth_mm / 2.0;
    double xOffset = xCenter-bounds.width() / 2.0;

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
    pathElem.setAttribute("style", "fill:black");

    return pathElem;
}


QDomElement SvgDom::makeQrCode(const QrCode &qr) {

    QDomElement qrElement = createElement("path");
    qrElement.setAttribute("fill", "#000000");
    qrElement.setAttribute("style", "stroke-width:1");
    QString matrix;
    QTextStream stream(&matrix);
    for (int y = 0; y < qr.getSize(); y++) {
        for (int x = 0; x < qr.getSize(); x++) {
            if (qr.getModule(x, y)) {
                if (x != 0 || y != 0)
                    stream << " ";
                stream << "M" << x << " " << y << "h1v1h-1z";
            }
        }
    }

    qrElement.setAttribute("d", matrix);
    return qrElement;
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
