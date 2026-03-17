#ifndef CUSTOMPRINTPREVIEW_H
#define CUSTOMPRINTPREVIEW_H

#include <QPageLayout>
#include <QPrintPreviewDialog>

class QPrintPreviewWidget;

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

    void storeSettings();
    void retrieveSettings();

protected:

protected slots:
    void zoom(const QString &zoomAsText);

private:
    QAction *findOrientationAction(const QString &iconName);
    QPrintPreviewWidget *preview = NULL;


};

#endif // CUSTOMPRINTPREVIEW_H
