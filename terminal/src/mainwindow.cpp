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

void MainWindow::on_open_file_triggered()
{
    QString file_name = QFileDialog::getOpenFileName(this, QString ("Открыть файл"), QString(), QString("xml (*.xml);; all (*.*)"));
        QFile file(file_name);
        if (file.open(QIODevice::ReadOnly))
        {
            ui->current_file_label->setText(file_name);
            QByteArray file_text = file.readAll();
            ui->textEdit->setText(file_text);
            file.close();
        }
}

void MainWindow::on_save_file_triggered()
{
    QFile file(ui->current_file_label->text());
    if (file.open(QIODevice::WriteOnly))
    {
        file.write(ui->textEdit->toPlainText().toUtf8());
        file.close();
    }
}

void MainWindow::on_save_as_triggered()
{
    QString file_name = QFileDialog::getSaveFileName(this, QString ("Сохранить файл"), QString(), QString("xml (*.xml)"));
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
