#ifndef IMAGELEGENDSMODEL_H
#define IMAGELEGENDSMODEL_H

#include <QAbstractListModel>
#include <QStandardItem>
#include <QTableView>
#include <QPixmap>
#include "rowtype.h"


class ImageLegendsModel : public QAbstractListModel {
    Q_OBJECT
    typedef QAbstractListModel Super;
public:

    explicit ImageLegendsModel(QObject *parent = nullptr);

    void awake(QTableView *tableView);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;


    bool insertRows(int position, int rows, const QModelIndex &index = QModelIndex()) override;
    bool removeRows(int position, int rows, const QModelIndex &index = QModelIndex()) override;

    void saveFile(const QString &fileName);
    void loadFile(const QString &fileName);


    Qt::DropActions supportedDropActions() const override;
    QStringList mimeTypes() const override;
    QMimeData *mimeData(const QModelIndexList &indexes) const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &modelIndex) override;
    
    void appendUrls(const QList<QUrl> &urls);
    
    RowType getRow(int row);
    
private:
    void appendUrl(QUrl &url);
    void inserRowsLocal(int position, int numberOfRows);
    QByteArray byteArray(const QString &fileName);

    RowType composeRow(const QByteArray &ba, const QString &urlString, int index) const;


public:
    QList<RowType> dataRows;

    
};

#endif // IMAGELEGENDSMODEL_H
