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

//Деструктор освобождающий ресурсы
ConfigDialog::~ConfigDialog()
{
    delete cd;
}

//Возвращение документ app из комбобокса.
Document *ConfigDialog::app() const
{
    return MainWindow::currentDocument(cd->appComboBox);
}

//Возвращение документ deploy из комбобокса.
Document *ConfigDialog::deploySchema() const
{
    return MainWindow::currentDocument(cd->deploySchemaComboBox);
}

//Добавление в комбобокс app документа
void ConfigDialog::loadApp(Document *doc)
{
    cd->appComboBox->addItem(doc->fileName(), QVariant::fromValue(doc));
}

//Добавление в комбобокс deploy документа
void ConfigDialog::loadDeploy(Document *doc)
{
    cd->deploySchemaComboBox->addItem(doc->fileName(), QVariant::fromValue(doc));
}

void ConfigDialog::on_buttonBox_accepted()
{
}

//Проверяет количество элементов к кобмбобоксах app и deploy
//Возвращает false если хотябы один комбобокс пуст.
bool ConfigDialog::ready() const
{
    return cd->appComboBox->count() > 0 && cd->deploySchemaComboBox->count() > 0;
}
