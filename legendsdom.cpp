#include <QMetaEnum>
//#include "logfile.h"
#include "legendsdom.h"

LegendsDom::LegendsDom() {

    clear();
    root = createElement("legends");
    appendChild(root);

}

void LegendsDom::addLine(const RowType &completeLine) {
    QDomElement line = createElement("line");
    QMetaEnum enumValue = QMetaEnum::fromType<RowType::ColumnType>();

    for (int column = 0; column < enumValue.keyCount(); column++) {
        const QString key = enumValue.valueToKey(column);
        QDomElement keyedText = createElement(key);
        keyedText.appendChild(createTextNode(completeLine.columnData(column).toString()));
        line.appendChild(keyedText);
    }

    root.appendChild(line);
}

const RowType LegendsDom::retrieveLine(const QDomNode &line) const {
    QString image;
    QString title;
    QStringList descriptions;
    QString camera;
    QString author;
    QString url;

    QMetaEnum enumValue = QMetaEnum::fromType<RowType::ColumnType>();
    QDomNodeList lineElements = line.childNodes();
    for (int j = 0; j < lineElements.count(); j++) {
        QDomNode n = lineElements.item(j);
        QDomElement e = n.toElement();
        switch (enumValue.keyToValue(e.tagName().toStdString().c_str())) {
        case RowType::Image:
            image = e.text();
            break;
        case RowType::Title:
            title = e.text();
            break;
        case RowType::Description:
            descriptions = e.text().split("\n");
            break;
        case RowType::Camera:
            camera = e.text();
            break;
        case RowType::Author:
            author = e.text();
            break;
        case RowType::Url_QR:
            url = e.text();
            break;
        }
    }

    return RowType(
                    image,
                    title,
                    descriptions,
                    camera,
                    author,
                    url
           );


}


