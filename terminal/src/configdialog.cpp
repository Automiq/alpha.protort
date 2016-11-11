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

void ConfigDialog::loadApp(QList<QString> c_app)
{
    cd->appComboBox->addItems(c_app);
}
void ConfigDialog::loadDeploy(QList<QString> c_deploy)
{
    cd->deploySchemaComboBox->addItems(c_deploy);
}

void ConfigDialog::on_buttonBox_accepted()
{
    this->hide();
}
