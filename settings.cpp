#include "settings.h"

Settings *Settings::shared = NULL;

void Settings::beginGroup(const QString &key) {

    Q_ASSERT(key.front().isUpper());
    settingsPtr->beginGroup(key);
    //qDebug() << "+" << settingsPtr->group();
}

void Settings::endGroup() {
    //qDebug() << "-" << settingsPtr->group();
    settingsPtr->endGroup();
}

/*
void Settings::clearBlocking() {
    blockSettings = 0;
}

bool Settings::blocked() {
    return blockSettings > 0;
}
*/

QVariant Settings::value(const QString &key, QVariant defaultValue) {
    Q_ASSERT(key.front().isUpper());
    //qDebug() << " " << key << "=" << settingsPtr->value(key, defaultValue);
    return settingsPtr->value(key, defaultValue);
}

void Settings::setValue(const QString &key, QVariant newValue) {
    //if (!blocked()) {
    //qDebug() << " " << key << ":" << newValue;
    settingsPtr->setValue(key, newValue);
    //}
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

void Settings::setAddPhotoInfo(bool showIt) {
    _addPhotoInfo = showIt;
}

bool Settings::addPhotoInfo() {
    return _addPhotoInfo;
}

void Settings::setAddCuttingLine(bool plotThem) {
    _addPlottingLines = plotThem;
}

bool Settings::addCuttingLine() {
    return _addPlottingLines;
}

void Settings::setWhiteOnBack(bool isInverse) {
    _whiteOnBlack = isInverse;
}

bool Settings::whiteOnBlack() {
    return _whiteOnBlack;
}

void Settings::setLabelAlignment(Qt::Alignment alignment) {
    _alignment = alignment;
}

Qt::Alignment Settings::labelAlignment() {
    return _alignment;
}

void Settings::storePageLayout(QPrinter *printer) {
    beginGroup("Print");

    QPageLayout layout = printer->pageLayout();

    setValue("Orientation", layout.orientation());
    setValue("PageSize", layout.pageSize().id());

    endGroup();
}

void Settings::retrievePageLayout(QPrinter *printer) {
    beginGroup("Print");

    int asInt = value("Orientation", printer->pageLayout().orientation()).toInt();
    QPageLayout::Orientation orientation = static_cast<QPageLayout::Orientation>(asInt);
    printer->setPageOrientation(orientation);
    int idAsInt =  value("PageSize", QPageSize::A4).toInt();
    QPageSize::PageSizeId id = static_cast<QPageSize::PageSizeId>(idAsInt);
    printer->setPageSize(QPageSize(id));

    endGroup();
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
    settingsPtr = new QSettings(APP_DOMAIN, APP_NAME);
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

    beginGroup("Settings");
    _defaultUrl = value("DefaultUrl", _defaultUrl).toString();
    _preserveCharacterSpacing = value("PreserveCharacterSpacing", _preserveCharacterSpacing).toBool();
    _dontUseNativeDialogs = value("DonUseNativeDialogs", _dontUseNativeDialogs).toBool();
    endGroup();

    beginGroup("Label");
    _addPhotoInfo = value("AddInfo", _addPhotoInfo).toBool();
    int alignmentAsInt = static_cast<int>(_alignment);
    alignmentAsInt = value("Alignment", alignmentAsInt).toInt();
    _alignment = static_cast<Qt::Alignment>(alignmentAsInt);
    _whiteOnBlack =value("WhiteOnBlack", _whiteOnBlack).toBool();
    endGroup();

    beginGroup("Plot");
    _addPlottingLines = value("CuttingLines", _addPlottingLines).toBool();
    endGroup();

}

void Settings::store() {

    beginGroup("Settings");
    setValue("PreserveCharacterSpacing", _preserveCharacterSpacing);
    setValue("UseNativeDialogs", _dontUseNativeDialogs);
    setValue("DefaultUrl", _defaultUrl);
    endGroup();

    beginGroup("Label");
    setValue("AddInfo", _addPhotoInfo);
    int alignmentAsInt = static_cast<int>(_alignment);
    setValue("Alignment", alignmentAsInt);
    setValue("WhiteOnBlack", _whiteOnBlack);
    endGroup();

    beginGroup("Plot");
    setValue("CuttingLines", _addPlottingLines);
    endGroup();

}

