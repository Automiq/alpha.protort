#include "configdialog.h"
#include "ui_configdialog.h"

ConfigDialog::ConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigDialog)
{
    ui->setupUi(this);
}

ConfigDialog::~ConfigDialog()
{
    delete ui;
}

QString ConfigDialog::app() const
{
    return ui->appComboBox->currentText();
}

QString ConfigDialog::deploySchema() const
{
    return ui->deploySchemaComboBox->currentText();
}

void ConfigDialog::on_buttonBox_accepted()
{
    this->hide();
}
