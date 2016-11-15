#include "configdialog.h"
#include "ui_configdialog.h"

ConfigDialog::ConfigDialog(QWidget *parent) :
    QDialog(parent),
    cd(new Ui::ConfigDialog)
{
    cd->setupUi(this);
}

QString ConfigDialog::app() const
{
    return cd->appComboBox->currentText();
}

QString ConfigDialog::deploySchema() const
{
    return cd->deploySchemaComboBox->currentText();
}

void ConfigDialog::loadApp(const QString &c_app)
{
    cd->appComboBox->addItem(c_app);
}

void ConfigDialog::loadDeploy(QString &c_deploy)
{
    cd->deploySchemaComboBox->addItem(c_deploy);
}

void ConfigDialog::on_buttonBox_accepted()
{
}
