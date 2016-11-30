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

Document *ConfigDialog::app() const
{
    QTextEdit *qte_ptr = qvariant_cast<QTextEdit *>(cd->appComboBox->currentData());
    return qobject_cast<Document *>(qte_ptr);
}

Document *ConfigDialog::deploySchema() const
{
    QTextEdit *qte_ptr = qvariant_cast<QTextEdit *>(cd->deploySchemaComboBox->currentData());
    return qobject_cast<Document *>(qte_ptr);
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
