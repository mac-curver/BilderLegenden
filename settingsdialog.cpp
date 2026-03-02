#include <QDialog>
#include <QMap>
#include <QString>
#include "mainwindow.h"
#include "settings.h"
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
    show();
}

void SettingsDialog::retrieveEditTriggers() {

    Settings::settings()->beginGroup("Settings");

    int editTrigger = Settings::settings()->value("EditTriggers", QAbstractItemView::CurrentChanged).toInt();
    static_cast<MainWindow *>(parent())->setEditTriggers(QAbstractItemView::EditTriggers(editTrigger));

    foreach (QCheckBox *checkbox, triggers.keys()) {
        bool checked = editTrigger & triggers[checkbox];
        qDebug() << editTrigger << checkbox->text() << triggers[checkbox] << checked;
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


    Settings::settings()->endGroup();
}

void SettingsDialog::storeEditTriggers() {
    Settings::settings()->beginGroup("Settings");

    QAbstractItemView::EditTriggers any = getCurrentCheckBoxes();
    Settings::settings()->setValue("EditTriggers", any.toInt());
    static_cast<MainWindow *>(parent())->setEditTriggers(any);

    Settings::settings()->endGroup();
}



QAbstractItemView::EditTriggers SettingsDialog::getCurrentCheckBoxes() {
    QAbstractItemView::EditTriggers any = QAbstractItemView::NoEditTriggers;
    foreach (QCheckBox *checkbox, triggers.keys()) {
        if (ui->noEditCheckBox != checkbox) {
            if (checkbox->isChecked()) {
                any |= triggers[checkbox];
            }
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
    QList<QCheckBox *> checkboxes = findChildren<QCheckBox *>();
    if (on) {
        foreach (QCheckBox *checkbox, checkboxes) {
            if (ui->noEditCheckBox != checkbox) {
                checkbox->setChecked(false);
            }
        }
    }
    storeEditTriggers();
}

void SettingsDialog::allEditTriggers(bool on) {

    QList<QCheckBox *> checkboxes = findChildren<QCheckBox *>();
    if (on) {
        foreach (QCheckBox *checkbox, checkboxes) {
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
}

void SettingsDialog::changeUseNativeDialogs(bool on) {
    Settings::shared->setDontUseNativeDialog(on);
}

void SettingsDialog::accept() {
    close();
}



void SettingsDialog::closeEvent(QCloseEvent *event) {
    Q_UNUSED(event)
    Settings::shared->setDefaultUrl(ui->defaultUrlLineEdit->text());

    Settings::shared->store();
}
