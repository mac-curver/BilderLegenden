#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QAbstractItemView>
#include <QSettings>
#include "settingsdialog.h"

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
    void printPdf();
    void exportSvg();
    void showAboutDialog();
    void showSettings();
    void setLabelSize(double );
    void setLabelFont(const QString &font);
    void setFrameThickness(double thickness_mm);
    void setOuterThickness(double thickness_mm);
    void recentTriggered(QAction* action);

protected slots:
    void onAboutToShowMainMenu();
    //virtual void resizeEvent(QResizeEvent *event) override;

private:
    void setStatus();
    void openPhoto(const QString &fileName);
    void loadWithName(const QString &legends);
    bool appendToRecent(QString fileName);
    void updateRecentMenu();
    void saveSvgFile(const QString &fileName, int pageNumberPlusOne);



private:
    Ui::MainWindow *ui;
    SettingsDialog *settingsDialog = NULL;
    QStringList recentPaths;
    QString _recentPhoto = "";
    
};
#endif // MAINWINDOW_H
