#include <QImageReader>
#include "aboutdialog.h"
#include "ui_aboutdialog.h"

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
}

AboutDialog::~AboutDialog() {
    delete ui;
}

bool AboutDialog::awake() {
    show();
    ui->title->setText(APP_NAME);
    ui->version->setText(QString("%1 co: %2").arg(APP_VERSION, APP_DOMAIN));
    QStringList thridParties = QStringList()
                 << "thanks to: "
                               << "- Nayuki for the QR Generator";

    thridParties << "- Oleh Aleinyk for the iptc-reader";
    thridParties << "    https://github.com/oaleynik/iptc-reader";
    thridParties << "- Trapezoid_Dreams for multiline edit";
    ui->thirdPartyTexts->appendPlainText("thanks to: ");
    ui->thirdPartyTexts->appendPlainText("- Nayuki for the QR Generator");
    ui->thirdPartyTexts->appendHtml("<a href=\"https://www.nayuki.io/page/qr-code-generator-library\">https://www.nayuki.io/page/qr-code-generator-library</a>");
    ui->thirdPartyTexts->appendPlainText("");

    ui->thirdPartyTexts->appendPlainText("- Mayank Lahiri for parts of the exif extractor");
    ui->thirdPartyTexts->appendPlainText("    Copyright (c) 2010-2015 Mayank Lahiri");
    ui->thirdPartyTexts->appendPlainText("    mlahiri@gmail.com");
    ui->thirdPartyTexts->appendPlainText("    All rights reserved (BSD License).");
    ui->thirdPartyTexts->appendPlainText("");

    ui->thirdPartyTexts->appendPlainText("- Oleh Aleinyk for the iptc-reader");
    ui->thirdPartyTexts->appendHtml("<a href=\"https://github.com/oaleynik/iptc-reader\">https://github.com/oaleynik/iptc-reader</a>");
    ui->thirdPartyTexts->appendPlainText("");

    ui->thirdPartyTexts->appendPlainText("- Trapezoid_Dreams for the multiline edit idea");


    return true;
}

void AboutDialog::accept() {
    this->deleteLater();
    close();
}


