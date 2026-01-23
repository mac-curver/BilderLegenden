#include <QPrintDialog>
#include <QLabel>
#include <QIcon>
#include <QSettings>
#include <QPainter>
//#include "logfile.h"
#include "mainwindow.h"
#include "svgmaindisplay.h"
#include "ui_svgmaindisplay.h"

SvgMainDisplay::SvgMainDisplay(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::SvgMainDisplay)
{

    ui->setupUi(this);

    pageSizeLabel = new QLabel("<p.size>");
    widthLabel = new QLabel("Width:");
    heightLabel = new QLabel("Height:");

    widthSpinBox = new QSpinBox(this);
    widthSpinBox->setSuffix(" mm");
    widthSpinBox->setMinimum(50);
    widthSpinBox->setMaximum(298);
    widthSpinBox->setAlignment(Qt::AlignRight);
    widthSpinBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    widthSpinBox->setMinimumSize(80,0);

    heightSpinBox = new QSpinBox(this);
    heightSpinBox->setSuffix(" mm");
    heightSpinBox->setMinimum(50);
    heightSpinBox->setMaximum(298);
    heightSpinBox->setAlignment(Qt::AlignRight);
    heightSpinBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    heightSpinBox->setMinimumSize(80,0);

    QSizeF labelSize_mm = ui->qrCodeWidget->labelSize_mm();
    widthSpinBox->setValue(labelSize_mm.width());
    heightSpinBox->setValue(labelSize_mm.height());

    ui->toolBar->insertWidget(ui->actionPrinterSetup, pageSizeLabel);
    ui->toolBar->insertSeparator(ui->actionPrinterSetup);
    ui->toolBar->insertWidget(ui->actionPrint, widthLabel);
    ui->toolBar->insertWidget(ui->actionPrint, widthSpinBox);
    ui->toolBar->insertWidget(ui->actionPrint, heightLabel);
    ui->toolBar->insertWidget(ui->actionPrint, heightSpinBox);

    ui->actionLandscape->setIcon(QIcon(":/icon/Images/Landscape.svg"));
    ui->actionPortrait->setIcon(QIcon(":/icon/Images/Portrait.svg"));
    setPageSetup();
    connect(widthSpinBox, &QSpinBox::valueChanged, this, &SvgMainDisplay::updateContentsSize);
    connect(heightSpinBox, &QSpinBox::valueChanged, this, &SvgMainDisplay::updateContentsSize);

    ui->scrollArea->setWidgetResizable(false);
    ui->qrCodeWidget->resizeContent();
}

SvgMainDisplay::~SvgMainDisplay() {
    delete ui;
}

void SvgMainDisplay::setLandscapeOrientation(bool isLandscape) {
    ui->qrCodeWidget->setLandscapeOrientation(isLandscape);
    ui->actionLandscape->setChecked(isLandscape);
    ui->actionPortrait->setChecked(!isLandscape);
}

void SvgMainDisplay::changePage(int page) {
    ui->qrCodeWidget->setPageNumber(page);
    ui->qrCodeWidget->update();
}

void SvgMainDisplay::storeSettings() {
    QSettings settings;
    settings.beginGroup("svgDisplay");
    settings.endGroup();
}

void SvgMainDisplay::retrieveSettings() {
    QSettings settings;
    settings.beginGroup("svgDisplay");
    settings.endGroup();
}

void SvgMainDisplay::setPageSetup() {

    ui->qrCodeWidget->printer->pageLayout().setUnits(QPageLayout::Millimeter);
    QPrinter *printer = ui->qrCodeWidget->printer;
    QPageLayout pageLayout = printer->pageLayout();
    qDebug() << "SvgMainDisplay::setPageSetup" << pageLayout << printer->resolution();
    qDebug() << printer->widthMM() << printer->heightMM();
    //QSizeF size = pageLayout->
    switch (pageLayout.orientation()) {
    case QPageLayout::Portrait:
        setLandscapeOrientation(false);
        break;
    default:
        setLandscapeOrientation(true);
        break;
    }
    //qDebug() << pageLayout << ui->qrCodeWidget->printer->resolution()/72.0 << "\"";
    // This is a much more reliable way to scale your output
    double scaleX_mm = ui->qrCodeWidget->printer->pageRect(QPrinter::Millimeter).width();
    double scaleY_mm = ui->qrCodeWidget->printer->pageRect(QPrinter::Millimeter).height();
    //ui->qrCodeWidget->setDomSize(QSizeF(scaleX_mm, scaleY_mm));
    //double scale_mm = fmin(scaleX_mm, scaleY_mm);
    pageSizeLabel->setText(QString("%1/%2,%3").arg(pageLayout.pageSize().name()).arg(scaleX_mm).arg(scaleY_mm));
}

void SvgMainDisplay::updateContentsSize(int ) {
    ui->qrCodeWidget->setLabelSize_mm(QSizeF(widthSpinBox->value(), heightSpinBox->value()));
    ui->qrCodeWidget->resizeContent();
}


void SvgMainDisplay::awake() {
    setWindowTitle("SVG Display");
    retrieveSettings();
    setGeometry(50, 50, width(), height());
}

void SvgMainDisplay::print(QPrinter *printerPtr) {
    doPageSetup();
    ui->qrCodeWidget->print(printerPtr);
}

void SvgMainDisplay::setImageLegends(ImageLegendsModel *modelPtr) {
    ui->qrCodeWidget->setImageLegends(modelPtr);
}

void SvgMainDisplay::showEvent(QShowEvent *event) {
    Q_UNUSED(event)
    Super::showEvent(event);
}

void SvgMainDisplay::closeEvent(QCloseEvent *event) {
    Q_UNUSED(event)
    static_cast<MainWindow*>(parent())->releaseShowButton();
    storeSettings();
}

void SvgMainDisplay::keyPressEvent(QKeyEvent *event) {
    qDebug() << "SvgMainDisplay::keyPressEvent";
    switch (event->key()) {
    case Qt::Key_Plus:
        ui->qrCodeWidget->zoomIn(1.2);
        break;
    case Qt::Key_Minus:
        ui->qrCodeWidget->zoomIn(1/1.2);
        break;
    case Qt::Key_Escape:
        ui->qrCodeWidget->resetZoom();
        break;
    default:
        QWidget::keyPressEvent(event);
        break;
    }
}



void SvgMainDisplay::setPageNumber(int page) {
    ui->qrCodeWidget->setPageNumber(page);
}


void SvgMainDisplay::setToPortrait(bool on) {
    setLandscapeOrientation(!on);
}

void SvgMainDisplay::setToLandscape(bool on) {
    setLandscapeOrientation(on);
}

void SvgMainDisplay::doPageSetup() {

    //if (NULL == printSetupDialog) {
    //    printSetupDialog = new QPrintPreviewDialog(ui->qrCodeWidget->printer, this);
    //    connect(printSetupDialog, &QPrintPreviewDialog::paintRequested, this, &SvgMainDisplay::printPreview);
    //}

    //printSetupDialog->setWindowTitle(tr("Print Legends"));
    //switch (printSetupDialog->exec()) {
    //case QDialog::Accepted:
    //    setPageSetup();
    //    break;
    //default:
    //    return;
    //}
}

void SvgMainDisplay::doPrint() {
    print(ui->qrCodeWidget->printer);
}

void SvgMainDisplay::printPreview(QPrinter *printer){
    QPainter painter(printer);
    painter.begin(printer);
    //QRect rect = painter.viewport();
    ui->qrCodeWidget->print(printer);
    //painter.drawPixmap(0, 0, buffer.scaled(rect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    painter.end();
}



