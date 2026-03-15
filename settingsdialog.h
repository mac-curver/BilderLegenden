#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QMap>
#include <QAbstractItemView>
#include <QCheckBox>
#include "propagatingtableview.h"

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT
    typedef QDialog Super;

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

    void awake();

    void retrieveEditTriggers(PropagatingTableView *tv);
    QString defaultUrl();
    

private:
    QAbstractItemView::EditTriggers getCurrentCheckBoxes();
    void storeEditTriggers();
    void updatePreview();


public slots:
    void editTriggerChanged(bool on);
    void noEditTrigger(bool on);
    void allEditTriggers(bool on);
    void changePreserveCharacterSpacing(bool on);
    void changeUseNativeDialogs(bool on);

    void changeAddInfo(bool on);
    void changeCuttingLines(bool plotThem);
    void changeWhiteOnBlack(bool inverse);


    void leftAlign();
    void centerAlign();
    void rightAlign();


protected:
    void accept() override;
    void closeEvent(QCloseEvent *event) override;

public:


private:
    QMap<QCheckBox *, QAbstractItemView::EditTriggers> triggers;
    PropagatingTableView *tableView = NULL;
    Ui::SettingsDialog *ui;
};

#endif // SETTINGSDIALOG_H
