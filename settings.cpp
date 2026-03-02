#include "settings.h"

Settings *Settings::shared = NULL;

QSettings *Settings::settings() {
    return shared->settingsPtr;
}

void Settings::setDefaultUrl(const QString &url) {
    _defaultUrl = url;
    store();
}

QString Settings::defaultUrl() {
    return _defaultUrl;
}

void Settings::setPreseveCharacterSpacing(bool preserve) {
    _preserveCharacterSpacing = preserve;
    store();
}

bool Settings::preserveCharacterSpacing() {
    return _preserveCharacterSpacing;
}

void Settings::setDontUseNativeDialog(bool native) {
    _dontUseNativeDialogs = native;
}


bool Settings::dontUseNativeDialog() {
    return _dontUseNativeDialogs;
}


QString Settings::svgFileName() {
    Q_ASSERT(false);
    return "";
}

QString Settings::pdfFileName() {
    Q_ASSERT(false);
    return "";
}


Settings::Settings() {
    settingsPtr = new QSettings(APP_COMPANY, APP_NAME);
    qDebug() << settingsPtr->fileName();
    qDebug() << settingsPtr->applicationName() << settingsPtr->organizationName() << settingsPtr->isWritable() << settingsPtr->fileName();
}

Settings::~Settings() {
    store();
    delete settingsPtr;
}


Settings *Settings::createSettings() {
    if (NULL == shared) {
        shared = new Settings();
        shared->retrieve();
    }
    return shared;
}

void Settings::retrieve() {

    settingsPtr->beginGroup("Settings");

    _defaultUrl = settingsPtr->value("DefaultUrl", _defaultUrl).toString();
    _preserveCharacterSpacing = settingsPtr->value("PreserveCharacterSpacing", _preserveCharacterSpacing).toBool();
    _dontUseNativeDialogs = settingsPtr->value("DonUseNativeDialogs", _dontUseNativeDialogs).toBool();

    settingsPtr->endGroup();
}

void Settings::store() {

    settingsPtr->beginGroup("Settings");

    settingsPtr->setValue("PreserveCharacterSpacing", _preserveCharacterSpacing);
    settingsPtr->setValue("UseNativeDialogs", _dontUseNativeDialogs);
    settingsPtr->setValue("DefaultUrl", _defaultUrl);

    settingsPtr->endGroup();
}
