#include <QDialog>
#include <QMap>
#include <QString>
#include <QSettings>
#include "mainwindow.h"
#include "settings.h"
#include "settingsdialog.h"
#include "ui_settingsdialog.h"

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    triggers[ui->noEditCheckBox] = QAbstractItemView::NoEditTriggers;
    triggers[ui->currentChangedCheckBox] = QAbstractItemView::CurrentChanged;
    triggers[ui->doubleClickedCheckBox] = QAbstractItemView::DoubleClicked;
    triggers[ui->selectedClickedCheckBox] = QAbstractItemView::SelectedClicked;
    triggers[ui->editKeyCheckBox] = QAbstractItemView::EditKeyPressed;
    triggers[ui->anyKeyCheckBox] = QAbstractItemView::AnyKeyPressed;
    triggers[ui->allEditCheckBox] = QAbstractItemView::AllEditTriggers;
}

SettingsDialog::~SettingsDialog() {
    delete ui;
}

void SettingsDialog::awake() {
    ui->defaultUrlLineEdit->setText(Settings::shared->defaultUrl());
    show();
}

void SettingsDialog::retrieveEditTriggers() {
    QSettings settings;

    settings.beginGroup("mainwindow");

    int editTrigger = settings.value("EditTriggers", QAbstractItemView::CurrentChanged).toInt();
    static_cast<MainWindow *>(parent())->setEditTriggers(QAbstractItemView::EditTriggers(editTrigger));

    foreach (QCheckBox *checkbox, triggers.keys()) {
        checkbox->setChecked(editTrigger & triggers[checkbox]);
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


    settings.endGroup();
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


void SettingsDialog::storeEditTriggers() {
    QSettings settings;

    settings.beginGroup("mainwindow");

    QAbstractItemView::EditTriggers any = getCurrentCheckBoxes();
    settings.setValue("EditTriggers", any.toInt());
    static_cast<MainWindow *>(parent())->setEditTriggers(any);

    settings.endGroup();
    qDebug() << "SettingsDialog::storeEditTriggers" << settings.fileName();
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



void SettingsDialog::closeEvent(QCloseEvent *event) {
    Q_UNUSED(event)
    Settings::shared->store();
}
