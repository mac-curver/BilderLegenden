#ifndef SETTINGS_H
#define SETTINGS_H


#include <QString>
#include <QSettings>
#include <QStandardPaths>

class Settings {
private:
    Settings();
    ~Settings();

public:
    static Settings *createSettings();

    void retrieve();

    void store();


    static Settings *shared;
    static QSettings *settings();

    void setDefaultUrl(const QString &url);
    QString defaultUrl();

    void setPreseveCharacterSpacing(bool preserve);
    bool preserveCharacterSpacing();

    QString svgFileName();
    QString pdfFileName();


private:
    QString defaultPdfFileName = "Test.pdf";
    QString defaultSvgFileName = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)+"/Legends.svg";
    QString _defaultUrl = "<https://www.fotocommunity.de>";
    QSettings *settingsPtr = NULL;
    bool _preserveCharacterSpacing = true;

};

#endif // SETTINGS_H
