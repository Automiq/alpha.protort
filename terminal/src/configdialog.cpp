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
}

ConfigDialog::~ConfigDialog()
{
    delete cd;
}

Document *ConfigDialog::app() const
{
    return MainWindow::current_doc(cd->appComboBox);
}

Document *ConfigDialog::deploySchema() const
{
    return MainWindow::current_doc(cd->deploySchemaComboBox);
}

void ConfigDialog::loadApp(Document *doc)
{
    cd->appComboBox->addItem(doc->fileName(), QVariant::fromValue(doc));
}

void ConfigDialog::loadDeploy(Document *doc)
{
    cd->deploySchemaComboBox->addItem(doc->fileName(), QVariant::fromValue(doc));
}

void ConfigDialog::on_buttonBox_accepted()
{
}

bool ConfigDialog::ready()
{
    return cd->appComboBox->count() > 0 && cd->deploySchemaComboBox->count() > 0;
}
