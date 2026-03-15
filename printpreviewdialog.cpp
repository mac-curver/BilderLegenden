
#include <QPrinter>
#include <QToolBar>
#include <QAction>
#include <QPrintPreviewWidget>
#include <QPainter>
#include "settings.h"
#include "printpreviewdialog.h"
#include "ui_printpreviewdialog.h"




PrintPreviewDialog::PrintPreviewDialog(QPrinter *printer, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PrintPreviewDialog)
    , m_printer(printer)
{
    ui->setupUi(this);

    // Connect preview widget to our render function
    connect(preview(), &QPrintPreviewWidget::paintRequested,
            this, &PrintPreviewDialog::renderPreview);

    setupToolbar();

    // Initial orientation state
    updateOrientationActions(preview()->orientation());
}

PrintPreviewDialog::~PrintPreviewDialog() {
    delete ui;
}

QToolBar *PrintPreviewDialog::toolBar() const {
    return ui->toolBar;
}

QPrintPreviewWidget *PrintPreviewDialog::preview() const {
    return ui->previewWidget;
}

void PrintPreviewDialog::setupToolbar() {
    auto tb = toolBar();

    // Zoom
    m_actZoomIn  = tb->addAction(tr("Zoom In"));
    m_actZoomOut = tb->addAction(tr("Zoom Out"));
    m_actFitWidth = tb->addAction(tr("Fit Width"));
    m_actFitPage  = tb->addAction(tr("Fit Page"));

    tb->addSeparator();

    // Orientation
    m_actPortrait  = tb->addAction(tr("Portrait"));
    m_actLandscape = tb->addAction(tr("Landscape"));

    m_actPortrait->setCheckable(true);
    m_actLandscape->setCheckable(true);

    // Connections
    connect(m_actZoomIn,  &QAction::triggered, this, &PrintPreviewDialog::zoomIn);
    connect(m_actZoomOut, &QAction::triggered, this, &PrintPreviewDialog::zoomOut);
    connect(m_actFitWidth,&QAction::triggered, this, &PrintPreviewDialog::fitWidth);
    connect(m_actFitPage, &QAction::triggered, this, &PrintPreviewDialog::fitPage);

    connect(m_actPortrait,  &QAction::triggered, this, &PrintPreviewDialog::setPortrait);
    connect(m_actLandscape, &QAction::triggered, this, &PrintPreviewDialog::setLandscape);
}

void PrintPreviewDialog::updateOrientationActions(QPageLayout::Orientation o) {
    if (!m_actPortrait || !m_actLandscape)
        return;

    m_actPortrait->setChecked(o == QPageLayout::Portrait);
    m_actLandscape->setChecked(o == QPageLayout::Landscape);
}

// --- Slots ---

void PrintPreviewDialog::renderPreview(QPrinter *printer) {
    // Example content: replace with your real drawing
    QPainter p(printer);
    p.drawText(100, 100, QStringLiteral("Hello from PrintPreviewDialog"));
}

void PrintPreviewDialog::setPortrait() {
    preview()->setOrientation(QPageLayout::Portrait);
    updateOrientationActions(QPageLayout::Portrait);
}

void PrintPreviewDialog::setLandscape() {
    preview()->setOrientation(QPageLayout::Landscape);
    updateOrientationActions(QPageLayout::Landscape);
}

void PrintPreviewDialog::zoomIn() {
    preview()->zoomIn();
}

void PrintPreviewDialog::zoomOut() {
    preview()->zoomOut();
}

void PrintPreviewDialog::fitWidth() {
    preview()->setZoomMode(QPrintPreviewWidget::FitToWidth);
}

void PrintPreviewDialog::fitPage() {
    preview()->setZoomMode(QPrintPreviewWidget::FitInView);
}

void PrintPreviewDialog::storeSettings() {
    QPageLayout layout = m_printer->pageLayout();

    Settings::settings()->beginGroup("Printer");
    Settings::settings()->setValue("Orientation", layout.orientation());
    Settings::settings()->endGroup();

}

void PrintPreviewDialog::retrieveSettings() {
    QPageLayout layout = m_printer->pageLayout();

    Settings::settings()->beginGroup("Printer");
    int asInt = Settings::settings()->value("Orientation", layout.orientation()).toInt();
    QPageLayout::Orientation orientation = static_cast<QPageLayout::Orientation>(asInt);
    layout.setOrientation(orientation);
    m_printer->setPageLayout(layout);

    //QAction *portraitAction  = findOrientationAction("qt_printpreview_portrait");
    /*QAction *landscapeAction = findOrientationAction("qt_printpreview_landscape");

    if (portraitAction && landscapeAction) {
        portraitAction->setChecked(orientation == QPageLayout::Portrait);
        landscapeAction->setChecked(orientation == QPageLayout::Landscape);
    }
    */

    Settings::settings()->endGroup();
}

void PrintPreviewDialog::updatePreview() {
    preview()->updatePreview();
}
