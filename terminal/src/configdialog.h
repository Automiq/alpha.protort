#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QDialog>
#include <QDialogButtonBox>
#include "document.h"

namespace Ui {
class ConfigDialog;
}

class ConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigDialog(QWidget *parent = 0);
    ~ConfigDialog();

    QString app() const;
    QString deploySchema() const;

    void loadApp(const QString &app);
    void loadDeploy(const QString &deploy);
    void setup();
    QDialogButtonBox *buttonBox2;
signals:
    void acptd();

public slots:
    void on_buttonBox_accepted();

private:
    Ui::ConfigDialog *cd;
};

#endif // ConfigDialog_H
