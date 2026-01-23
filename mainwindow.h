#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QAbstractItemView>
//#include "qrcodewidget.h"
#include "settingsdialog.h"
//#include "imagelegendsmodel.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
    typedef QMainWindow Super;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    bool awake();

    void releaseShowButton();

    void setEditTriggers(QAbstractItemView::EditTriggers triggers);

protected:
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;


    //void dragEnterEvent(QDragEnterEvent *event) override;
    //void dragMoveEvent(QDragMoveEvent *event) override;

    //void dragLeaveEvent(QDragLeaveEvent *event) override;
    //void dropEvent(QDropEvent *event) override;

public slots:
    void save();
    void load();
    void insertRow();
    void removeRow();
    void addPhoto();
    void updateTableView();
    void printPreview();
    //void printSvg();
    void showAboutDialog();
    void showSettings();
    void setLabelSize(double );
    void setFrameThickness(double thickness_mm);

private:
    void setStatus();
    void openPhoto(const QString &fileName);



private:
    Ui::MainWindow *ui;
    SettingsDialog *settingsDialog = NULL;

    
};
#endif // MAINWINDOW_H
