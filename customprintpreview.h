#ifndef CUSTOMPRINTPREVIEW_H
#define CUSTOMPRINTPREVIEW_H

#include <QPrintPreviewDialog>

class CustomPrintPreview: public QPrintPreviewDialog {
    typedef QPrintPreviewDialog Super;
    Q_OBJECT
public:

    explicit CustomPrintPreview(QPrinter *printer, QWidget *parent = nullptr,
                                 Qt::WindowFlags flags = Qt::WindowFlags());
    ~CustomPrintPreview();

    void accept() override;
    //void reject() override;

    void updatePreview();



};

#endif // CUSTOMPRINTPREVIEW_H
