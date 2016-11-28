#include "configdialog.h"
#include "ui_configdialog.h"
#include <QFileInfo>
#include <QDialogButtonBox>

ConfigDialog::ConfigDialog(QWidget *parent) :
    QDialog(parent),
    cd(new Ui::ConfigDialog)
{
    cd->setupUi(this);
}

ConfigDialog::~ConfigDialog()
{
    delete cd;
}

QString ConfigDialog::app() const
{
    return cd->appComboBox->currentText();
}

QString ConfigDialog::deploySchema() const
{
    return cd->deploySchemaComboBox->currentText();
}

void ConfigDialog::loadApp(const QString &app)
{
    cd->appComboBox->addItem(QFileInfo(app).fileName());
}

void ConfigDialog::loadDeploy(const QString &deploy)
{
    cd->deploySchemaComboBox->addItem(QFileInfo(deploy).fileName());
}

void ConfigDialog::on_buttonBox_accepted()
{
}
