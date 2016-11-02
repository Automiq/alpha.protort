#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_save_file_triggered()
{
    QFile file(ui->tabWidget->tabText(ui->tabWidget->currentIndex()));
    if (file.open(QIODevice::WriteOnly))
    {
        file.write(ui->textEdit->toPlainText().toUtf8());
        file.close();
    }
}

void MainWindow::on_save_all_triggered()
{
    QString file_name = QFileDialog::getSaveFileName(this, QString ("Сохранить файл"), QString(), QString("xml (*.xml);; all (*)"));
    QFile file(file_name);
    if (file.open(QIODevice::WriteOnly))
    {
        file.write(ui->textEdit->toPlainText().toUtf8());
        file.close();
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
        if (ui->tabWidget->tabText(ui->tabWidget->currentIndex()) == "Tab 1")
        {
            ui->tabWidget->setTabText(ui->tabWidget->currentIndex(), QString(QFileInfo(file_name).fileName()));
            QByteArray file_text = file.readAll();
            ui->textEdit->setText(file_text);
            file.close();
        }
        else
        {
            QTextEdit *text_edit = new QTextEdit();
            ui->tabWidget->setCurrentIndex(ui->tabWidget->addTab(text_edit, QString(QFileInfo(file_name).fileName())));
            ui->tabWidget->setTabText(ui->tabWidget->currentIndex(), QString(QFileInfo(file_name).fileName()));
            QByteArray file_text = file.readAll();
            text_edit->setText(file_text);
            file.close();
        }
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
