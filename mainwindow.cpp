#include <QPrinter>
#include <QPrintDialog>
#include <QPrintPreviewWidget>
#include <QMessageBox>
#include <QStandardPaths>
#include <QFileDialog>
#include <QMimeData>
#include <QSettings>
#include <QAbstractItemView>

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

    QSettings settings;


    if (qApp->queryKeyboardModifiers() & Qt::ShiftModifier) {
        qDebug() << "Shiftkey was down";
    }
    else {
        settings.beginGroup("mainwindow");
        if (!isFullScreen()) {
            settings.setValue("size", size());
        }
        QSize newSize = settings.value("size", QSize(600, 400)).toSize();
        QPoint newPos = settings.value("position", QSize(100, 50)).toPoint();
        resize(newSize);
        move(newPos);
        bool shouldBeFullScreen = settings.value("fullScreen", false).toBool();
        settings.endGroup();

        if (shouldBeFullScreen) {
            settings.setValue("size", size());
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


    settings.beginGroup("labelSettings");
    ui->widthDoubleSpinBox->setValue(settings.value("width", ui->widthDoubleSpinBox->value()).toDouble());
    ui->heightDoubleSpinBox->setValue(settings.value("height", ui->heightDoubleSpinBox->value()).toDouble());
    ui->frameDoubleSpinBox->setValue(settings.value("frameThickness", ui->frameDoubleSpinBox->value()).toDouble());
    settings.endGroup();

    // Does not Work!? but for file menu it works ?!
    //connect(ui->actionPreferences->menu(), &QMenu::aboutToShow, this, &MainWindow::onAboutToShowMainMenu);

    setAcceptDrops(true);

    //svgDisplay = new SvgMainDisplay(this);
    //svgDisplay->awake();
    //svgDisplay->setImageLegends(&legendsModel);
    //svgDisplay->setVisible(ui->showPushButton->isChecked());
    //ui->tableView->setSvgDisplay(svgDisplay);

    connect(ui->tableView, &PropagatingTableView::requestUpdate, this, &MainWindow::updateTableView);

    //insertRows(-1, 4);

    show();
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

void MainWindow::save() {
    qDebug() << "MainWindow::awake()";
    QString fileName=QStandardPaths::writableLocation(QStandardPaths::HomeLocation)+"/legends.xml";
    QFileDialog fileDialog(this);
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    if (!fileName.isEmpty()) {
        fileDialog.setDirectory(QFileInfo(fileName).absolutePath());
    }
    fileDialog.selectFile(fileName);
    switch (fileDialog.exec()) {
    case QDialog::Accepted:
        fileName = fileDialog.selectedFiles().at(0);
        static_cast<ImageLegendsModel*>(ui->tableView->model())->saveFile(fileName);
        break;
    default:
        break;
    }

}

void MainWindow::load() {
    qDebug() << "MainWindow::load()";
    QString fileName=QStandardPaths::standardLocations(QStandardPaths::HomeLocation).constFirst()+"/legends.xml";
    QFileDialog fileDialog(this);
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    if (!fileName.isEmpty()) {
        fileDialog.setDirectory(QFileInfo(fileName).absolutePath());
    }
    fileDialog.selectFile(fileName);
    switch (fileDialog.exec()) {
    case QDialog::Accepted:
        fileName = fileDialog.selectedFiles().at(0);
        static_cast<ImageLegendsModel*>(ui->tableView->model())->loadFile(fileName);
        break;
    default:
        break;
    }
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
    QString fileName = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).constFirst()+"/";
    QFileDialog fileDialog(this);
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    if (!fileName.isEmpty()) {
        fileDialog.setDirectory(QFileInfo(fileName).absolutePath());
    }
    fileDialog.selectFile(fileName);
    switch (fileDialog.exec()) {
    case QDialog::Accepted:
        fileName = fileDialog.selectedFiles().at(0);
        openPhoto(fileName);
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
    settingsDialog->setVisible(true);
}

void MainWindow::closeEvent(QCloseEvent *event) {
    QSettings settings;

    settings.beginGroup("mainwindow");
    settings.setValue("fullScreen", isFullScreen());
    if (!isFullScreen()) {
        settings.setValue("position", pos());
        settings.setValue("size", size());
    }
    settings.endGroup();

    Super::closeEvent(event);
    qApp->quit();
}

void MainWindow::setLabelSize(double) {
    QSize size_mm(ui->widthDoubleSpinBox->value(), ui->heightDoubleSpinBox->value());
    ui->tableView->setLabelSize_mm(size_mm);

    QSettings settings;

    settings.beginGroup("labelSettings");
    settings.setValue("width", size_mm.width());
    settings.setValue("height", size_mm.height());
    settings.endGroup();
}

void MainWindow::setFrameThickness(double thickness_mm) {
    ui->tableView->setFrameThickness(thickness_mm);

    QSettings settings;

    settings.beginGroup("labelSettings");
    settings.setValue("frameThickness", thickness_mm);
    settings.endGroup();

}


