#include "settings.h"

Settings *Settings::shared = NULL;


Settings::Settings() {

}

Settings *Settings::createSettings() {
    if (NULL == shared) {
        shared = new Settings();
        shared->retrieve();
    }
    return shared;
}

void Settings::retrieve() {
    QSettings settings;

    settings.beginGroup("Settings");

    _defaultUrl = settings.value("DefaultUrl", _defaultUrl).toString();

    settings.endGroup();
}

void Settings::store() {
    QSettings settings;

    settings.beginGroup("Settings");

    settings.setValue("DefaultUrl", _defaultUrl);

    settings.endGroup();
}
