#ifndef SVGDISPLAY_H
#define SVGDISPLAY_H

#include <QMainWindow>
#include <QPrinter>
#include "imagelegendsmodel.h"

namespace Ui {
class SvgDisplay;
}

class SvgDisplay : public QMainWindow
{
    Q_OBJECT
    typedef QMainWindow Super;

public:
    explicit SvgDisplay(QWidget *parent = nullptr);
    ~SvgDisplay();

    void print(QPrinter *printerPtr);

    void setImageLegends(ImageLegendsModel *modelPtr);
    void updateNumLines();


protected:
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

public slots:
    void updateContents();
    void setPageNumber(int page);
    void createPrinter();

protected slots:


private:
    Ui::SvgDisplay *ui;
    int _pageNumber = 0;
};

#endif // SVGDISPLAY_H
