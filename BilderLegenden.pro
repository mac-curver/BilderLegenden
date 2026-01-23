QT       += core gui widgets svg svgwidgets xml printsupport


CONFIG += c++17

VERSION   = 2.01.00

#    win32 {
#        COMMON_PATH=$$PWD/../../_Common
#    } mac {
#        COMMON_PATH=$$PWD/../../GitQtLocal/Mac_Common
#    }


mac {
    DEFINES += USE_TOUCH
    ICON = Images/BilderLegendenIcon.icns # must be added to project (created with GC)
}

win {
    RC_ICON = Images/BilderLegenden.ICO
}


DEFINES += \
    APP_COMPANY=\\\"LegoEsprit\\\"      \
    APP_DOMAIN=\\\"LegoEsprit.de\\\"    \
    APP_NAME=\\\"$$TARGET\\\"           \
    APP_VERSION=\\\"$$VERSION\\\"


# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
#    $$COMMON_PATH/logFile.cpp \
    aboutdialog.cpp \
    centereddelegate.cpp \
    cpp/qrcodegen.cpp \
    customprintpreview.cpp \
    exif.cpp \
    heicxmlreader.cpp \
    imagelegendsmodel.cpp \
    iptc.cpp \
    legendsdom.cpp \
    main.cpp \
    mainwindow.cpp \
    multilinedelegate.cpp \
    propagatingtableview.cpp \
    qrcodewidget.cpp \
    rowtype.cpp \
    settings.cpp \
    settingsdialog.cpp \
    #svgdisplay.cpp \
    svgdom.cpp \
#    svgmaindisplay.cpp \
#    testwidget.cpp

HEADERS += \
#    $$COMMON_PATH/logfile.h \
    aboutdialog.h \
    centereddelegate.h \
    cpp/qrcodegen.hpp \
    customprintpreview.h \
    exif.h \
    heicxmlreader.h \
    imagelegendsmodel.h \
    iptc.h \
    legendsdom.h \
    mainwindow.h \
    multilinedelegate.h \
    propagatingtableview.h \
    qrcodewidget.h \
    rowtype.h \
    settings.h \
    settingsdialog.h \
    #svgdisplay.h \
    svgdom.h \
#    svgmaindisplay.h \
#    testwidget.h

INCLUDEPATH += \
#    $$COMMON_PATH


FORMS += \
    aboutdialog.ui \
    mainwindow.ui \
    settingsdialog.ui \
#    svgmaindisplay.ui

RESOURCES += \
    resources.qrc


DISTFILES += \
    Images/BilderLegendenIcon.icns \
    Images/BilderLegendenIcon.ico \
    Windeploy.bat

