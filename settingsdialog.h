#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QMap>
#include <QAbstractItemView>
#include <QCheckBox>

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

    void retrieveEditTriggers();
    QString defaultUrl();
    

private:
    QAbstractItemView::EditTriggers getCurrentCheckBoxes();
    void storeEditTriggers();


public slots:
    void editTriggerChanged(bool on);
    void noEditTrigger(bool on);
    void allEditTriggers(bool on);

protected:
    void closeEvent(QCloseEvent *event);

public:


private:
    QMap<QCheckBox *, QAbstractItemView::EditTriggers> triggers;
    Ui::SettingsDialog *ui;
};

#endif // SETTINGSDIALOG_H
