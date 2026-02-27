#include <QDebug>
#include <QString>
#include <QFile>
#include <QUrl>
#include <QMetaEnum>
#include <QMetaType>
#include <QMimeData>
#include <QDomDocument>
#include <QMessageBox>

//#include "logfile.h"
#include "settings.h"
#include "exif.h"
#include "iptc.h"
#include "heicxmlreader.h"

#include "rowtype.h"
#include "imagelegendsmodel.h"
#include "legendsdom.h"

ImageLegendsModel::ImageLegendsModel(QObject *parent)
    : QAbstractListModel{parent}
{
}


int ImageLegendsModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return dataRows.count();
}

int ImageLegendsModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return RowType::ColumnType::N_ColumnTypes;
}


QVariant ImageLegendsModel::data(const QModelIndex &index, int role) const {
    if (index.isValid() &&  index.row() < dataRows.count()) {
        int row = index.row();
        int column = index.column();
        switch (role) {
        case Qt::DisplayRole:
            switch (column) {
            case RowType::ColumnType::Image:
                if (dataRows.at(row).containsBitmap()) {
                    return QVariant();
                }
                else {
                    return dataRows.at(row).columnData(column);
                }
            default:
                return dataRows.at(row).columnData(column);
            }
        case Qt::EditRole:
            return dataRows.at(row).columnData(column);
        case Qt::DecorationRole:
            return dataRows.at(row).columnDecoration(column);
        case Qt::ForegroundRole:
            return dataRows.at(row).columnColor(column);
        default:
            return QVariant();
        }
    }
    else {
        return QVariant();
    }

}

QVariant ImageLegendsModel::headerData(int section, Qt::Orientation orientation, int role) const {
    switch (role) {
    case Qt::DisplayRole:
        switch (orientation) {
        case Qt::Horizontal:
            return QVariant::fromValue(static_cast<RowType::ColumnType>(section)).toString();
        default:
            if (section < dataRows.count()) {
                return QString("%1").arg(section+1);
            }
            else {
                return QVariant();
            }
        }
    case Qt::BackgroundRole:
        return QColor(0xFFFFE0);

    default:
        return QVariant();
    }

}

Qt::ItemFlags ImageLegendsModel::flags(const QModelIndex &index) const {

    //Qt::ItemFlag dropEnable = Qt::ItemIsDropEnabled;
    Qt::ItemFlag dropEnable = Qt::NoItemFlags;

    switch (index.column()) {

    case RowType::ColumnType::Image:
        return Qt::ItemIsEnabled | dropEnable | Qt::ItemIsDragEnabled;
    default:
        if (index.isValid()) {
            return QAbstractItemModel::flags(index) | Qt::ItemIsEditable | dropEnable | Qt::ItemIsDragEnabled;
        }
        return Qt::ItemIsEnabled;
    }

}

bool ImageLegendsModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (index.isValid() && role == Qt::EditRole) {
        int column = index.column();
        RowType *rowPtr = &dataRows[index.row()];
        rowPtr->setColumnData(column, value);
        emit dataChanged(index, index, {role});

        return true;
    }
    return false;
}


void ImageLegendsModel::inserRowsLocal(int position, int numberOfRows) {

    for (int row = 0; row < numberOfRows; ++row) {
        int lineNumber = qMax(0, position) + row + 1;
        RowType *completeRow = new RowType(
            "",     // No image link
            QString("<Titel %1>").arg(lineNumber),
            QStringList() << QString("<mehrzeilige Beschreibung %1>").arg(lineNumber),
            "",
            QString("<Autor %1>").arg(lineNumber),
            Settings::shared->defaultUrl()
        );
        if (0 > position) {
            dataRows.append(*completeRow);
        }
        else {
            dataRows.insert(position, *completeRow);
        }
    }
}

QByteArray ImageLegendsModel::byteArray(const QString &fileName) {
    QByteArray ba;

    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly)) {
        ba = file.readAll();
        file.close();
    }
    return ba;
}


bool ImageLegendsModel::insertRows(int position, int numberOfRows, const QModelIndex &index) {
    beginInsertRows(index, position, position+numberOfRows-1);

    inserRowsLocal(position, numberOfRows);

    endInsertRows();
    return true;

}

void ImageLegendsModel::appendUrl(QUrl &url) {
    QByteArray ba = byteArray(url.toLocalFile());
    RowType exifRow = composeRow(ba, url.toLocalFile(), dataRows.count() + 1);
    dataRows.append(exifRow);
}

void ImageLegendsModel::appendUrls(const QList<QUrl> &urls) {
    beginInsertRows(QModelIndex(), rowCount(), rowCount() + urls.count() - 1);
    foreach (QUrl url, urls) {
        appendUrl(url);
    }
    // qDebug() << lineNumber << dataRows.count();
    endInsertRows();
}

bool ImageLegendsModel::removeRows(int position, int rows, const QModelIndex &index) {
    if (rowCount() > 0) {
        int removePosition = qMin(position, rowCount()-1);
        beginRemoveRows(index, removePosition, removePosition+rows-1);

        for (int row = 0; row < rows; ++row) {
            dataRows.removeAt(removePosition);
        }

        endRemoveRows();
        return true;
    }
    else {
        return false;
    }
}

void ImageLegendsModel::saveFile(const QString &fileName) {
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        LegendsDom doc;
        for (int row = 0; row < dataRows.count(); row++) {
            RowType completeRow = dataRows.at(row);
            doc.addLine(completeRow);
        }
        QTextStream stream(&file);
        stream << doc.toString() << Qt::endl;
        file.close();
    }
}

void ImageLegendsModel::loadFile(const QString &fileName) {
    qDebug() << "ImageLegendsModel::loadFile" << fileName;
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        layoutAboutToBeChanged();

        LegendsDom doc;
        doc.setContent(&file);

        file.close();

        QDomElement docElem = doc.documentElement();

        QDomNode legends = docElem.firstChild();
        QDomNodeList nodeList = docElem.elementsByTagName("line");
        qDebug() << "ImageLegendsModel::loadFile" << nodeList.count();
        dataRows.clear();
        for (int i = 0; i < nodeList.count(); i++ ) {
            QDomNode line = nodeList.item(i);
            dataRows.append(doc.retrieveLine(line));
        }
        layoutChanged();
    }
}


Qt::DropActions ImageLegendsModel::supportedDropActions() const {
    return Qt::CopyAction;// | Qt::MoveAction;
}

QStringList ImageLegendsModel::mimeTypes() const {
    QStringList types;
    types << "text/plain";
    types << "text/uri-list";
    types << "application/vnd.text.list";
    return types;
}

QMimeData *ImageLegendsModel::mimeData(const QModelIndexList &indexes) const {
    QMimeData *mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    foreach (const QModelIndex &index, indexes) {
        if (index.isValid()) {
            QString text = data(index, Qt::DisplayRole).toString();
            stream << text;
        }
    }
    mimeData->setData("application/vnd.text.list", encodedData);
    return mimeData;

}


bool ImageLegendsModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                                     int row, int column, const QModelIndex &modelIndex
) {
    Q_UNUSED(column)
    qDebug() << "ImageLegendsModel::dropMimeData" << action << data->formats();
    if (action != Qt::IgnoreAction) {

        if (data->hasFormat("application/vnd.text.list")) {

            int beginRow;

            if (row != -1) {
                beginRow = row;
            }
            else if (modelIndex.isValid()) {
                beginRow = modelIndex.row();
            }
            else {
                beginRow = rowCount(QModelIndex());
            }
            QByteArray encodedData = data->data("application/vnd.text.list");
            QDataStream stream(&encodedData, QIODevice::ReadOnly);
            QStringList newItems;
            int rows = 0;

            while (!stream.atEnd()) {
                QString text;
                stream >> text;
                newItems << text;
                ++rows;
            }
            insertRows(beginRow, rows, QModelIndex());
            foreach (const QString &text, newItems) {
                QModelIndex idx = index(beginRow, 0, QModelIndex());
                setData(idx, text);
                beginRow++;
            }
        }
        else if (data->hasFormat("text/uri-list")) {
            QUrl url = data->text();
            if (url.isLocalFile()) {
                QByteArray ba = byteArray(url.toLocalFile());
                if (dataRows.at(row).columnData(RowType::Title).toString().isEmpty()) {

                    dataRows[row].setColumnData(RowType::Title, url.fileName());
                }
                if (dataRows.at(row).columnData(RowType::Camera).toString().isEmpty()) {
                    QString camera = "?????";
                    dataRows[row].setColumnData(RowType::Camera, camera);
                }
                dataRows[row].setColumnData(RowType::Image, url.toLocalFile());
            }
            else {
                QMessageBox::critical(static_cast<QWidget*>(parent()->parent()), "No Exif file", "Can't find Exif information.");
            }

            return true;
        }
        return false;
    }
    else  {
        return true;
    }

}



RowType ImageLegendsModel::composeRow(const QByteArray &ba, const QString &urlString, int index) const {
    QStringList cameraList;

    easyexif::EXIFInfo exif(ba);
    IPTC iptc(ba);
    HeicXmlReader heic(ba);


    QString title = iptc.titleAsString().c_str();
    if (title.isEmpty()) {
        title = heic.titleAsString().c_str();
        if (title.isEmpty()) {
            title = QString("<Titel %1>").arg(index);
        }
    }
    title = title.trimmed();
    QString camera = exif.cameraAsString().c_str();
    if (camera.isEmpty()) {
        camera = heic.cameraAsString().c_str();
        if (camera.isEmpty()) {
            camera = iptc.cameraAsString().c_str();
        }
    }
    QStringList descriptionList;
    QString descriptionText = exif.descriptionAsString().c_str();
    if (descriptionText.isEmpty()) {
        descriptionText = heic.descriptionAsString().c_str();
        if (descriptionText.isEmpty()) {
            descriptionText = iptc.descriptionAsString().c_str();
            if (descriptionText.isEmpty()) {
                descriptionText = QString("<Description %1>").arg(index);
            }
        }
    }
    descriptionList = descriptionText.split('\n');

    QString author = exif.creatorAsString().c_str();
    if (author.isEmpty()) {
        author = heic.creatorAsString().c_str();
        if (author.isEmpty()) {
            author = iptc.creatorAsString().c_str();
            if (author.isEmpty()) {
                if (author.isEmpty()) {
                    author = QString("<Author %1>").arg(index);
                }
            }
        }
    }
    author = author.trimmed();
    QString urlLink = iptc.linkLocation().c_str();
    if (urlLink.isEmpty()) {
        urlLink = heic.linkLocation().c_str();
    }
    urlLink = urlLink.trimmed();

    return RowType(urlString, title, descriptionList, camera, author, urlLink);
}


RowType ImageLegendsModel::getRow(int row) {
    return row < dataRows.count() ? dataRows.at(row) : RowType("", "Unknown", QStringList() << "", "", "", "");
}

