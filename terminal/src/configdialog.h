#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QDialog>
#include "document.h"

namespace Ui {
class ConfigDialog;
}

class ConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigDialog(QWidget *parent = 0);
    QString app() const;
    QString deploySchema() const;

    void loadApp(QString &c_app);
    void loadDeploy(QString &c_deploy);

private slots:
    void on_buttonBox_accepted();

private:
    Ui::ConfigDialog *cd;
};

#endif // ConfigDialog_H
