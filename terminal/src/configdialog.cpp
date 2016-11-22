#include "configdialog.h"
#include "ui_configdialog.h"
#include "mainwindow.h"
#include <QFileInfo>
#include <QDialogButtonBox>

ConfigDialog::ConfigDialog(QWidget *parent) :
    QDialog(parent),
    cd(new Ui::ConfigDialog)
{
    cd->setupUi(this);
    setup();
}

void ConfigDialog::setup()
{
    buttonBox2 = new QDialogButtonBox(QDialogButtonBox::Ok
                                     | QDialogButtonBox::Cancel);
    cd->verticalLayout_2->addWidget(buttonBox2);
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
    emit acptd();
}
