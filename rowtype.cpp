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

const QVariant RowType::columnColor(int column) const {
    switch (static_cast<ColumnType>(column)) {
    case Image:
        return QVariant();
    default:
        return underEdit[column] ? QColor(Qt::blue) : QColor(Qt::black);
    }
}

bool RowType::containsBitmap() const {
    return !pixMap.isNull();
}

void RowType::setColumnData(int column, const QVariant &text) {
    ColumnType columnType = static_cast<ColumnType>(column);
    switch (columnType) {
    case Image:
        if (text.toString() != imageLink) {
            imageLink = text.toString();
            pixMap = createPixmap();
        }
        break;
    case Title:
        title = text.toString();
        underEdit[columnType] = false;
        break;
    case Description:
        descriptions = text.toString().split("\n");
        underEdit[columnType] = false;
        break;
    case Camera:
        camera = text.toString();
        underEdit[columnType] = false;
        break;
    case Author:
        author = text.toString();
        underEdit[columnType] = false;
        break;
    case Url_QR:
        url = text.toString();
        underEdit[columnType] = false;
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

void RowType::enterEditMode(int column) {
    ColumnType columnType = static_cast<ColumnType>(column);
    underEdit[columnType] = true;
}

void RowType::exitEditMode(int column) {
    ColumnType columnType = static_cast<ColumnType>(column);
    underEdit[columnType] = false;
}




