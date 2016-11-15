#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "protoclient.h"
#include "parser.h"

#include <QFileDialog>
#include <QTextEdit>
#include <QObject>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

//    createNewTab("New");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_save_file_triggered()
{
    auto text_edit = dynamic_cast<QTextEdit*> (ui->tabWidget->currentWidget());
    QFile file(ui->tabWidget->tabText(ui->tabWidget->currentIndex()));
    if (file.open(QIODevice::ReadWrite))
    {
        file.write(text_edit->toPlainText().toUtf8());
        file.close();
    }
}

void MainWindow::on_save_all_triggered()
{
    for (int i = ui->tabWidget->count(); i >= 0; --i)
    {
        auto text_edit = dynamic_cast<QTextEdit*> (ui->tabWidget->widget(i));
        if (!text_edit)
            continue;
        QFile file(ui->tabWidget->tabText(i));
        if (file.open(QIODevice::WriteOnly))
        {
            file.write(text_edit->toPlainText().toUtf8());
            file.close();
        }
    }
}

void MainWindow::on_exit_triggered()
{
    close(); //добавить остановку приложения
}

void MainWindow::on_load_file_triggered()
{
    QString file_name = QFileDialog::getOpenFileName(this, QString ("Открыть файл"), QString(), QString("xml (*.xml);; all (*)"));
    QFile file(file_name);
    if (file.open(QIODevice::ReadOnly))
    {
        auto text_edit = createNewTab(QString(QFileInfo(file_name).fileName()));
        QByteArray file_text = file.readAll();
        text_edit->setText(file_text);
        file.close();
    }
}

void MainWindow::on_create_file_triggered()
{
    QString file_name = QFileDialog::getSaveFileName(this, QString ("Создать файл"), QString(), QString("xml (*.xml);; all (*)"));
    QFile file(file_name);
    if (file.open(QIODevice::ReadWrite))
    {
        if (ui->tabWidget->tabText(ui->tabWidget->currentIndex()) == "Tab 1")
            ui->tabWidget->setTabText(ui->tabWidget->currentIndex(), QString(QFileInfo(file_name).fileName()));
        else
            ui->tabWidget->setCurrentIndex(ui->tabWidget->addTab(new QTextEdit(), QString(QFileInfo(file_name).fileName())));
        file.close();
    }
}

void MainWindow::on_tabWidget_tabCloseRequested(int index)
{
    ui->tabWidget->currentWidget()->deleteLater();
    ui->tabWidget->removeTab(index);
    if (ui->tabWidget->count() == 0)
        close();
}

QTextEdit* MainWindow::createNewTab(const QString &name)
{
    QTextEdit *text_edit = new QTextEdit();
    ui->tabWidget->setCurrentIndex(ui->tabWidget->addTab(text_edit, name));
    ui->tabWidget->setTabText(ui->tabWidget->currentIndex(), name);
    return text_edit;
}

void MainWindow::on_deploy_triggered()
{

}
