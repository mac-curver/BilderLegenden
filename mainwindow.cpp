#include <QPrinter>
#include <QPainter>
#include <QPrintDialog>
#include <QPrintPreviewWidget>
#include <QMessageBox>
#include <QStandardPaths>
#include <QFileDialog>
#include <QMimeData>
#include <QSettings>
#include <QAbstractItemView>
#include <QFontComboBox>
#include <QSaveFile>
#include <QInputDialog>

//#include "logfile.h"
#include "settings.h"
//#include "customprintpreview.h"
#include "propagatingtableview.h"
#include "aboutdialog.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    Settings::createSettings();
    ui->setupUi(this);
}

MainWindow::~MainWindow() {
    delete ui;
}

bool MainWindow::awake() {

    Settings::settings()->beginGroup("labelSettings");
    QString test = Settings::settings()->value("font", "emptyfont").toString();
    Settings::settings()->endGroup();

    if (qApp->queryKeyboardModifiers() & Qt::ShiftModifier) {
        qDebug() << "Shiftkey was down";
    }
    else {
        Settings::settings()->beginGroup("mainwindow");
        if (!isFullScreen()) {
            Settings::settings()->setValue("size", size());
        }
        QSize newSize = Settings::settings()->value("size", QSize(600, 400)).toSize();
        QPoint newPos = Settings::settings()->value("position", QSize(100, 50)).toPoint();
        resize(newSize);
        move(newPos);
        bool shouldBeFullScreen = Settings::settings()->value("fullScreen", false).toBool();
        Settings::settings()->endGroup();

        if (shouldBeFullScreen) {
            Settings::settings()->setValue("size", size());
            showFullScreen();
        }
        else {
            showNormal();
        }
    }


    if (NULL == settingsDialog) {
        settingsDialog = new SettingsDialog(this);
    }
    settingsDialog->retrieveEditTriggers();


    Settings::settings()->beginGroup("labelSettings");

    ui->fontComboBox->setFontFilters(QFontComboBox::ScalableFonts);
    QString settingsFontAsString = Settings::settings()->value("font", ui->fontComboBox->currentFont()).toString();
    ui->fontComboBox->setCurrentFont(QFont(settingsFontAsString));
    ui->tableView->setLabelFont(settingsFontAsString);

    ui->widthDoubleSpinBox->setValue(Settings::settings()->value("width", ui->widthDoubleSpinBox->value()).toDouble());
    ui->heightDoubleSpinBox->setValue(Settings::settings()->value("height", ui->heightDoubleSpinBox->value()).toDouble());
    ui->frameDoubleSpinBox->setValue(Settings::settings()->value("frameThickness", ui->frameDoubleSpinBox->value()).toDouble());
    Settings::settings()->endGroup();

    // Does not Work!? but for file menu it works ?!
    //connect(ui->actionPreferences->menu(), &QMenu::aboutToShow, this, &MainWindow::onAboutToShowMainMenu);

    setAcceptDrops(true);

    //svgDisplay = new SvgMainDisplay(this);
    //svgDisplay->awake();
    //svgDisplay->setImageLegends(&legendsModel);
    //svgDisplay->setVisible(ui->showPushButton->isChecked());
    //ui->tableView->setSvgDisplay(svgDisplay);

    recentPaths = Settings::settings()->value("RecentFiles").toStringList();
    updateRecentMenu();


    if (!recentPaths.empty()) {
        QString legendsFileName = recentPaths.last();
        loadWithName(legendsFileName);
    }

    connect(ui->tableView, &PropagatingTableView::requestUpdate, this, &MainWindow::updateTableView);

    //insertRows(-1, 4);

    show();

    QString testTranslation = QObject::tr("German");
    qDebug() << testTranslation;

    return true;
}

void MainWindow::showEvent(QShowEvent *event) {
    Super::showEvent(event);
}

/*
void MainWindow::dragEnterEvent(QDragEnterEvent *event)  {
    LOGIN("%s", event->mimeData()->formats().join(":").constData());
    if (event->mimeData()->hasFormat("text/plain")) {
        event->acceptProposedAction();
    }
    LOGOUT("");
}

void MainWindow::dragMoveEvent(QDragMoveEvent *event) {
    LOGIN("");
    event->acceptProposedAction();
    LOGOUT("");
}

void MainWindow::dragLeaveEvent(QDragLeaveEvent *event) {
    LOGIN("");
    event->accept();
    LOGOUT("");
}

void MainWindow::dropEvent(QDropEvent *event) {
    Q_UNUSED(event)
    LOGIN("");
    //QPoint pos = QPoint(event->position().x(), event->position().y());
    //QModelIndex index = ui->tableView->indexAt(pos);
    LOGOUT("");
}
*/

bool MainWindow::appendToRecent(QString fileName) {
    bool exists = recentPaths.removeOne(fileName);

    recentPaths << fileName;
    for (;recentPaths.length() > 10; ) {
        recentPaths.takeFirst();
    }
    updateRecentMenu();
    return exists;
}

void MainWindow::updateRecentMenu() {
    ui->menuRecent_Projects->clear();
    for (auto item = recentPaths.cbegin(), end = recentPaths.cend(); item != end; ++item) {
        QAction *action = new QAction(*item);
        ui->menuRecent_Projects->insertAction(NULL, action);
    }
}



void MainWindow::save() {
    qDebug() << "MainWindow::save()";
    QFileDialog fileDialog(this);
    qDebug() << Settings::shared->dontUseNativeDialog();
    fileDialog.setOption(QFileDialog::DontUseNativeDialog, Settings::shared->dontUseNativeDialog());
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    QString fileName;
    if (recentPaths.isEmpty()) {
        fileName = QStandardPaths::writableLocation(QStandardPaths::HomeLocation)+"/legends.xml";
    }
    else {
        fileName = recentPaths.last();
    }
    if (!fileName.isEmpty()) {
        fileDialog.setDirectory(QFileInfo(fileName).absolutePath());
    }
    fileDialog.selectFile(fileName);
    switch (fileDialog.exec()) {
    case QDialog::Accepted:
        if (!fileDialog.selectedFiles().isEmpty()) {
            fileName = fileDialog.selectedFiles().at(0);
            static_cast<ImageLegendsModel*>(ui->tableView->model())->saveFile(fileName);

            appendToRecent(fileName);
            Settings::settings()->setValue("RecentFiles", recentPaths);
        }
        break;
    default:
        break;
    }

}

void MainWindow::load() {
    qDebug() << "MainWindow::load()";
    QString fileName=QStandardPaths::standardLocations(QStandardPaths::HomeLocation).constFirst()+"/legends.xml";
    QFileDialog fileDialog(this);
    fileDialog.setOption(QFileDialog::DontUseNativeDialog, Settings::shared->dontUseNativeDialog());
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    if (!fileName.isEmpty()) {
        fileDialog.setDirectory(QFileInfo(fileName).absolutePath());
    }
    fileDialog.selectFile(fileName);
    switch (fileDialog.exec()) {
    case QDialog::Accepted:
        fileName = fileDialog.selectedFiles().at(0);
        loadWithName(fileName);
        break;
    default:
        break;
    }
}

void MainWindow::loadWithName(const QString &legends) {
    static_cast<ImageLegendsModel*>(ui->tableView->model())->loadFile(legends);
    appendToRecent(legends);
}



void MainWindow::setStatus() {
    QRectF r = ui->tableView->getPageRect();
    QPageLayout l = ui->tableView->getPageLayout();
    double scaleX_mm = r.width();
    double scaleY_mm = r.height();
    ui->statusbar->showMessage(QString("%1/%2,%3").arg(l.pageSize().name()).arg(scaleX_mm).arg(scaleY_mm));
}


void MainWindow::insertRow() {
    QModelIndexList selectedIndices = ui->tableView->selectionModel()->selectedIndexes();
    int selectedRow = ui->tableView->model()->rowCount();
    if (!selectedIndices.isEmpty()) {
        QModelIndex firstIndex = selectedIndices.first();
        selectedRow = firstIndex.row();
    }
    ui->tableView->model()->insertRows(selectedRow, 1);
    ui->tableView->update();
    ui->tableView->updatePreview();
}

void MainWindow::removeRow() {
    QModelIndexList selectedIndices = ui->tableView->selectionModel()->selectedIndexes();
    int selectedRow = ui->tableView->model()->rowCount();
    if (!selectedIndices.isEmpty()) {
        QModelIndex firstIndex = selectedIndices.first();
        selectedRow = firstIndex.row();
    }
    ui->tableView->model()->removeRows(selectedRow, 1);
    ui->tableView->updatePreview();
}

void MainWindow::openPhoto(const QString &fileName) {
    QList<QUrl> photoUrls;
    photoUrls << QUrl::fromLocalFile(fileName);
    static_cast<ImageLegendsModel*>(ui->tableView->model())->appendUrls(photoUrls);
    //ui->tableView->update();
    ui->tableView->updatePreview();
}


void MainWindow::addPhoto() {
    qDebug() << "MainWindow::addPhoto()";
    QFileDialog fileDialog(this);
    fileDialog.setOption(QFileDialog::DontUseNativeDialog, Settings::shared->dontUseNativeDialog());
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    if (_recentPhoto.isEmpty()) {
        _recentPhoto = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).constFirst()+"/";
    }
    fileDialog.selectFile(_recentPhoto);
    switch (fileDialog.exec()) {
    case QDialog::Accepted:
        _recentPhoto = fileDialog.selectedFiles().at(0);
        openPhoto(_recentPhoto);
        break;
    default:
        break;
    }
}

void MainWindow::updateTableView() {
    ui->tableView->updatePreview();
}


void MainWindow::printPreview() {
    setStatus();
    ui->tableView->showPrintPreview(true);
}

void MainWindow::printPdf() {
    QPrinter printer(QPrinter::PrinterResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);

    QString fileName = QStandardPaths::writableLocation(QStandardPaths::HomeLocation)+"/Legends.pdf";
    QFileDialog fileDialog(this);
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    if (!fileName.isEmpty()) {
        fileDialog.setDirectory(QFileInfo(fileName).absolutePath());
    }
    fileDialog.selectFile(fileName);
    QPainter painter(&printer);
    bool success;
    switch (fileDialog.exec()) {
    case QDialog::Accepted:
        fileName = fileDialog.selectedFiles().at(0);
        printer.setOutputFileName(fileName);
        success = printer.setPageMargins(QMarginsF(12, 16, 12, 20), QPageLayout::Millimeter);
        ui->tableView->codeWidget->render(&painter);
        qDebug() << success;
        break;
    default:
        break;
    }

}

void MainWindow::saveSvgFile(const QString &fileName, int pageNumberPlusOne) {
    QSaveFile file(fileName);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(ui->tableView->codeWidget->svg(pageNumberPlusOne));
        file.commit();
    }
}

void MainWindow::exportSvg() {

    bool ok{};
    int pageNumberPlusOne = QInputDialog::getInt(this, tr("Get page number (1-99)"), tr("Page"),
        1, 1, 100, 1,
        &ok
    );
    if (ok) {
        QString name = QString("/Legends%1.svg").arg(pageNumberPlusOne);
        QString fileName = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)+name;
        QFileDialog fileDialog(this);
        fileDialog.setAcceptMode(QFileDialog::AcceptSave);
        if (!fileName.isEmpty()) {
            fileDialog.setDirectory(QFileInfo(fileName).absolutePath());
        }
        fileDialog.selectFile(fileName);
        switch (fileDialog.exec()) {
        case QDialog::Accepted:
            saveSvgFile(fileDialog.selectedFiles().at(0), pageNumberPlusOne);
            break;
        default:
            break;
        }
    }

}


void MainWindow::releaseShowButton() {
    //ui->showPushButton->setChecked(false);
}


void MainWindow::setEditTriggers(QAbstractItemView::EditTriggers triggers) {
    ui->tableView->setEditTriggers(triggers);
}


//void MainWindow::printSvg() {
//    svgDisplay->doPrint();
//}

void MainWindow::showAboutDialog() {
    AboutDialog *about = new AboutDialog(this);
    about->awake();
}

void MainWindow::showSettings() {
    settingsDialog->awake();
}

void MainWindow::closeEvent(QCloseEvent *event) {

    Settings::settings()->beginGroup("mainwindow");
    Settings::settings()->setValue("fullScreen", isFullScreen());
    if (!isFullScreen()) {
        Settings::settings()->setValue("position", pos());
        Settings::settings()->setValue("size", size());
    }
    Settings::settings()->endGroup();
    int status = Settings::settings()->status();
    if (0 != status) {
        qDebug() << Settings::settings()->status();
    }

    Super::closeEvent(event);
    qApp->quit();
}

void MainWindow::setLabelSize(double) {
    QSize size_mm(ui->widthDoubleSpinBox->value(), ui->heightDoubleSpinBox->value());
    ui->tableView->setLabelSize_mm(size_mm);

    Settings::settings()->beginGroup("labelSettings");
    Settings::settings()->setValue("width", size_mm.width());
    Settings::settings()->setValue("height", size_mm.height());
    Settings::settings()->endGroup();
}

void MainWindow::setLabelFont(const QString &font) {
    ui->tableView->setLabelFont(font);

    Settings::settings()->beginGroup("labelSettings");
    Settings::settings()->setValue("font", font);
    Settings::settings()->endGroup();

}


void MainWindow::setFrameThickness(double thickness_mm) {
    ui->tableView->setFrameThickness(thickness_mm);

    Settings::settings()->beginGroup("labelSettings");
    Settings::settings()->setValue("frameThickness", thickness_mm);
    Settings::settings()->endGroup();

}

void MainWindow::recentTriggered(QAction *action) {
    loadWithName(action->text());
}


