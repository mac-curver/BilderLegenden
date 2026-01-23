#include <QDebug>
#include "rowtype.h"


const QVariant RowType::columnData(int column) const {
    switch (static_cast<ColumnType>(column)) {
    case Image:
        return imageLink;
    case Title:
        return title;
    case Description:
        return descriptions.join("\n");
    case Camera:
        return camera;
    case Author:
        return author;
    case Url_QR:
        return url;
    default:
        return QVariant();
    }
}

const QVariant RowType::columnDecoration(int column) const {
    switch (static_cast<ColumnType>(column)) {
    case Image:
        return pixMap;
    default:
        return QVariant();
    }
}

bool RowType::containsBitmap() const {
    return !pixMap.isNull();
}

void RowType::setColumnData(int column, const QVariant &text) {
    switch (static_cast<ColumnType>(column)) {
    case Image:
        if (text.toString() != imageLink) {
            imageLink = text.toString();
            pixMap = createPixmap();
        }
        break;
    case Title:
        title = text.toString();
        break;
    case Description:
        descriptions = text.toString().split("\n");
        break;
    case Camera:
        camera = text.toString();
        break;
    case Author:
        author = text.toString();
        break;
    case Url_QR:
        url = text.toString();
        break;
    default:
        break;
    }

}

QPixmap RowType::createPixmap() const  {
    QPixmap original = QPixmap(imageLink);
    if (original.isNull()) {
        return QPixmap();
    }
    else {
        QPixmap pmp = original.scaled(QSize(70, 70), Qt::KeepAspectRatio);
        return pmp;
    }
}



