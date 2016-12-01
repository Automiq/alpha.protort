#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QComboBox>
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

    Document *app() const;
    Document *deploySchema() const;

    void loadApp(Document *doc);
    void loadDeploy(Document *doc);
    bool ready();

private slots:
    void on_buttonBox_accepted();

private:
    Ui::ConfigDialog *cd;
};

#endif // ConfigDialog_H
