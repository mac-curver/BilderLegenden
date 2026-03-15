#ifndef SETTINGS_H
#define SETTINGS_H


#include <QString>
#include <QSettings>
#include <QStandardPaths>
#include <QPrinter>
#include <QPageLayout>

class Settings {
private:
    Settings();
    ~Settings();

public:

    static Settings *createSettings();

    void retrieve();

    void store();

    bool readOnly() {
        return _readOnly;
    }

    void setReadOnly() {
        _readOnly = true;
    }

    void clearReadOnly() {
        _readOnly = false;
    }



    static Settings *shared;
    //static QSettings *settings();
    void beginGroup(const QString &key);

    void endGroup();

    //void clearBlocking();
    //bool blocked();

    QVariant value(const QString &key, QVariant defaultValue = QVariant());
    void setValue(const QString &key, QVariant newValue);



    void setDefaultUrl(const QString &url);
    QString defaultUrl();

    void setPreseveCharacterSpacing(bool preserve);
    bool preserveCharacterSpacing();

    void setDontUseNativeDialog(bool native);
    bool dontUseNativeDialog();

    void setAddPhotoInfo(bool showIt);
    bool addPhotoInfo();

    void setAddCuttingLine(bool plotThem);
    bool addCuttingLine();

    void setWhiteOnBack(bool isInverse);
    bool whiteOnBlack();

    void setLabelAlignment(Qt::Alignment alignment);
    Qt::Alignment labelAlignment();

    void storePageLayout(QPrinter *printer);

    void retrievePageLayout(QPrinter *printer);

    QString svgFileName();
    QString pdfFileName();


private:
    QSettings *settingsPtr = NULL;
    bool _readOnly = false;

    QString defaultPdfFileName = "Test.pdf";
    QString defaultSvgFileName = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)+"/Legends.svg";
    QString _defaultUrl = "<https://www.fotocommunity.de>";

    bool _preserveCharacterSpacing = true;
    bool _dontUseNativeDialogs = false;
    bool _addPhotoInfo = false;
    bool _addPlottingLines = true;
    bool _whiteOnBlack = false;
    Qt::Alignment _alignment = Qt::AlignHCenter; // Qt::AlignLeft, Qt::AlignRight

};

#endif // SETTINGS_H
