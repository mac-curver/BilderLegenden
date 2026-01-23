#ifndef ROWTYPE_H
#define ROWTYPE_H

#include <QObject>
#include <QString>
#include <QPixmap>
#include <QVariant>


struct RowType {
    Q_GADGET
public:
    enum ColumnType {
        Image,
        Title,
        Description,
        Camera,
        Author,
        Url_QR,
        N_ColumnTypes
    };
    Q_ENUM(ColumnType)

    RowType(
            const QString &image,
            const QString &title,
            const QStringList &descriptions,
            const QString &camera,
            const QString &author,
            const QString &url
    ) {
        this->imageLink = image;
        this->title = title;
        this->descriptions = descriptions;
        this->camera = camera;
        this->author = author;
        this->url = url;

        this->pixMap = createPixmap();
    }

    /*
    RowType(
        const QString &title,
        const QString &camera,
        const QString &url
    ) {
        this->imageLink = "";
        this->title = title;
        this->descriptions = QStringList();
        this->camera = camera;
        this->author = "";
        this->url = url;

        this->pixMap = QPixmap();
    }
    */



    const QVariant columnData(int column) const;
    const QVariant columnDecoration(int column) const;

    bool containsBitmap() const;

    void setColumnData(int column, const QVariant &text);

    QPixmap createPixmap() const;


public:
    QString imageLink;
    QString title;
    QStringList descriptions;
    QString camera;
    QString author;
    QString url;

    QPixmap pixMap = QPixmap();


};


#endif // ROWTYPE_H
