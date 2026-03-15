#ifndef PRINTPREVIEWDIALOG_H
#define PRINTPREVIEWDIALOG_H

#pragma once

#include <QDialog>
#include <QPageLayout>

class QPrinter;
class QToolBar;
class QAction;
class QPrintPreviewWidget;

namespace Ui {
    class PrintPreviewDialog;
}

class PrintPreviewDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PrintPreviewDialog(QPrinter *printer, QWidget *parent = nullptr);
    ~PrintPreviewDialog() override;

    void updatePreview();
    void storeSettings();
    void retrieveSettings();

private slots:
    void renderPreview(QPrinter *printer);
    void setPortrait();
    void setLandscape();
    void zoomIn();
    void zoomOut();
    void fitWidth();
    void fitPage();

private:
    void setupToolbar();
    void updateOrientationActions(QPageLayout::Orientation o);


private:
    Ui::PrintPreviewDialog *ui;
    QPrinter *m_printer;

    QAction *m_actPortrait = nullptr;
    QAction *m_actLandscape = nullptr;
    QAction *m_actZoomIn = nullptr;
    QAction *m_actZoomOut = nullptr;
    QAction *m_actFitWidth = nullptr;
    QAction *m_actFitPage = nullptr;

    QToolBar *toolBar() const;
    QPrintPreviewWidget *preview() const;




};

#endif // PRINTPREVIEWDIALOG_H
