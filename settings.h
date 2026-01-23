#ifndef SETTINGS_H
#define SETTINGS_H


#include <QString>
#include <QSettings>

class Settings {
public:
    Settings();


    static Settings *createSettings();

    void retrieve();

    void store();


    static Settings *shared;

    void setDefaultUrl(const QString &url) {
        _defaultUrl = url;
        store();
    }

    QString defaultUrl() {
        return _defaultUrl;
    }


private:
    QString _defaultUrl = "<https://www.fotocommunity.de>";

};

#endif // SETTINGS_H
