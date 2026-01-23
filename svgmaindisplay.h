#ifndef SVGMAINDISPLAY_H
#define SVGMAINDISPLAY_H

#include <QMainWindow>
#include <QPrinter>
#include <QSpinBox>
#include <QLabel>
#include <QPrintDialog>
//#include <QPrintPreviewDialog>
#include "imagelegendsmodel.h"

namespace Ui {
class SvgMainDisplay;
}

class SvgMainDisplay: public QMainWindow
{
    Q_OBJECT
    typedef QMainWindow Super;

public:
    explicit SvgMainDisplay(QWidget *parent = nullptr);
    ~SvgMainDisplay();

    void awake();

    void print(QPrinter *printerPtr);

    void setImageLegends(ImageLegendsModel *modelPtr);

    void setLandscapeOrientation(bool isLandscape);
    void changePage(int page);


protected:
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    void storeSettings();
    void retrieveSettings();
    void setPageSetup();


public slots:
    void updateContentsSize(int size);
    void setPageNumber(int page);
    void setToPortrait(bool on);
    void setToLandscape(bool on);
    void doPageSetup();
    void doPrint();
    void printPreview(QPrinter *printer);


protected slots:


private slots:

private:
    Ui::SvgMainDisplay *ui;
    QSpinBox *widthSpinBox = NULL;
    QSpinBox *heightSpinBox = NULL;
    QLabel *pageSizeLabel = NULL;
    QLabel *widthLabel = NULL;
    QLabel *heightLabel = NULL;

    //QPrintPreviewDialog *printSetupDialog = NULL;

};



#endif // SVGMAINDISPLAY_H
