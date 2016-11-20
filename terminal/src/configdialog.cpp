#include "configdialog.h"
#include "ui_configdialog.h"

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
    cd->appComboBox->addItem(app);
}

void ConfigDialog::loadDeploy(const QString &deploy)
{
    cd->deploySchemaComboBox->addItem(deploy);
}

void ConfigDialog::on_buttonBox_accepted()
{
    //Здесь вызывается signal setupConfig(),который меняет текущую конфигурацию в тул баре на новую,
    //+значки для соответствующей конфигурации меняются на активные
}
