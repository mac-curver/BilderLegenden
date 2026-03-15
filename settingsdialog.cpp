#include <QDialog>
#include <QMap>
#include <QString>
#include "mainwindow.h"
#include "settings.h"
#include "propagatingtableview.h"
#include "settingsdialog.h"
#include "ui_settingsdialog.h"

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
     // must not be here as set implicitely below!!! triggers[ui->noEditCheckBox] = QAbstractItemView::NoEditTriggers;
    triggers[ui->currentChangedCheckBox] = QAbstractItemView::CurrentChanged;
    triggers[ui->doubleClickedCheckBox] = QAbstractItemView::DoubleClicked;
    triggers[ui->selectedClickedCheckBox] = QAbstractItemView::SelectedClicked;
    triggers[ui->editKeyCheckBox] = QAbstractItemView::EditKeyPressed;
    triggers[ui->anyKeyCheckBox] = QAbstractItemView::AnyKeyPressed;
    // must not be here !!! triggers[ui->allEditCheckBox] = QAbstractItemView::AllEditTriggers;
}

SettingsDialog::~SettingsDialog() {
    delete ui;
}

void SettingsDialog::awake() {
    ui->defaultUrlLineEdit->setText(Settings::shared->defaultUrl());
    ui->preserveCharacterSpacingCheckBox->setChecked(Settings::shared->preserveCharacterSpacing());
    ui->dontUseNativeDialogsCheckBox->setChecked(Settings::shared->dontUseNativeDialog());
    ui->showParametersCheckBox->setChecked(Settings::shared->addPhotoInfo());
    ui->plotCuttingCheckBox->setChecked(Settings::shared->addCuttingLine());
    ui->whiteOnBlackCheckBox->setChecked(Settings::shared->whiteOnBlack());
    switch (Settings::shared->labelAlignment()) {
    case Qt::AlignRight:
        ui->rightRadioButton->setChecked(true);
        break;
    case Qt::AlignLeft:
        ui->leftRadioButton->setChecked(true);
        break;
    default:
        ui->leftRadioButton->setChecked(true);
    }
    show();
}

void SettingsDialog::retrieveEditTriggers(PropagatingTableView *tv) {

    tableView = tv;
    Settings::shared->beginGroup("Settings");

    int editTrigger = Settings::shared->value("EditTriggers", QAbstractItemView::CurrentChanged).toInt();
    tableView->setEditTriggers(QAbstractItemView::EditTriggers(editTrigger));

    foreach (QCheckBox *checkbox, triggers.keys()) {
        bool checked = editTrigger & triggers[checkbox];
        checkbox->setChecked(checked);
    }

    switch (editTrigger) {
    case QAbstractItemView::NoEditTriggers:
        ui->noEditCheckBox->setChecked(true);
        break;
    case QAbstractItemView::AllEditTriggers:
        ui->allEditCheckBox->setChecked(true);
        break;
    default:
        /*
        {
            QList<QCheckBox *> checkboxes = findChildren<QCheckBox *>();
            foreach (QAbstractItemView::EditTriggers trigger, triggers) {
                if (editTrigger & trigger) {
                    checkboxes[i] = editTrigger & trigger;
                }
            }
        }
        */
        break;
    }

    Settings::shared->endGroup();
}

void SettingsDialog::storeEditTriggers() {
    Settings::shared->beginGroup("Settings");

    QAbstractItemView::EditTriggers any = getCurrentCheckBoxes();
    Settings::shared->setValue("EditTriggers", any.toInt());
    static_cast<MainWindow *>(parent())->setEditTriggers(any);

    Settings::shared->endGroup();
}

void SettingsDialog::updatePreview() {
    if (tableView) {
        tableView->updatePreview();
    }
}



QAbstractItemView::EditTriggers SettingsDialog::getCurrentCheckBoxes() {
    QAbstractItemView::EditTriggers any = QAbstractItemView::NoEditTriggers;
    foreach (QCheckBox *checkbox, triggers.keys() ) {
        if (checkbox->isChecked()) {
            any |= triggers[checkbox];
        }
    }
    switch (any) {
    case QAbstractItemView::NoEditTriggers:
        ui->noEditCheckBox->setChecked(true);
        break;
    case QAbstractItemView::AllEditTriggers:
        ui->allEditCheckBox->setChecked(true);
        break;
    default:
        break;
    }

    return any;
}



void SettingsDialog::editTriggerChanged(bool on) {
    Q_UNUSED(on)

    if (on) {
        ui->noEditCheckBox->setChecked(false);
    }
    else {
        ui->allEditCheckBox->setChecked(false);
    }

    storeEditTriggers();
}


void SettingsDialog::noEditTrigger(bool on) {
    if (on) {
        foreach (QCheckBox *checkbox, triggers.keys()) {
            if (ui->noEditCheckBox != checkbox) {
                checkbox->setChecked(false);
            }
        }
    }
    storeEditTriggers();
}

void SettingsDialog::allEditTriggers(bool on) {

    if (on) {
        foreach (QCheckBox *checkbox, triggers.keys()) {
            if (ui->noEditCheckBox != checkbox) {
                checkbox->setChecked(true);
            }
        }
        ui->noEditCheckBox->setChecked(false);
    }
    storeEditTriggers();
}

void SettingsDialog::changePreserveCharacterSpacing(bool on) {
    Settings::shared->setPreseveCharacterSpacing(on);
    updatePreview();
}

void SettingsDialog::changeUseNativeDialogs(bool on) {
    Settings::shared->setDontUseNativeDialog(on);
}

void SettingsDialog::changeAddInfo(bool on) {
    Settings::shared->setAddPhotoInfo(on);
    updatePreview();
}

void SettingsDialog::changeCuttingLines(bool plotThem) {
    Settings::shared->setAddCuttingLine(plotThem);
    updatePreview();
}

void SettingsDialog::changeWhiteOnBlack(bool inverse) {
    Settings::shared->setWhiteOnBack(inverse);
    updatePreview();
}

void SettingsDialog::leftAlign() {
    Settings::shared->setLabelAlignment(Qt::AlignLeft);
    updatePreview();
}

void SettingsDialog::centerAlign() {
    Settings::shared->setLabelAlignment(Qt::AlignHCenter);
    updatePreview();
}

void SettingsDialog::rightAlign() {
    Settings::shared->setLabelAlignment(Qt::AlignRight);
    updatePreview();
}

void SettingsDialog::accept() {
    close();
    updatePreview();
}



void SettingsDialog::closeEvent(QCloseEvent *event) {
    Q_UNUSED(event)
    Settings::shared->setDefaultUrl(ui->defaultUrlLineEdit->text());

    Settings::shared->store();
}
