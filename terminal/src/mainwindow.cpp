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

//void MainWindow::on_load_scheme_triggered()
//{
//    QFile app("app.xml");
//    if (app.open(QIODevice::ReadWrite))
//    {
//        QByteArray app_text = app.readAll();
//        ui->textEdit->setText(app_text);
//        app.close();
//    }
//    else
//        ui->textEdit->setText("ERROR! Cannot open app.xml");
//}

//void MainWindow::on_load_descripton_triggered()
//{
//    QFile app("app.xml");
//    if (app.open(QIODevice::ReadWrite))
//    {
//        QByteArray app_text = app.readAll();
//        ui->textEdit->setText(app_text);
//        app.close();
//    }
//    else
//        ui->textEdit->setText("ERROR! Cannot open app.xml");
//}

void MainWindow::on_open_file_triggered()
{
    QString file_name = QFileDialog::getOpenFileName(this, QString ("Открыть файл"), QString(), QString("xml (*.xml);; all (*.*)"));
        QFile file(file_name);
        if (file.open(QIODevice::ReadWrite))
        {
            ui->current_file->setText(file_name);
            QByteArray file_text = file.readAll();
            ui->textEdit->setText(file_text);
            file.close();
        }
}

void MainWindow::on_save_file_triggered()
{
    QFile file("app2.xml");
    if (file.open(QIODevice::WriteOnly))
    {
        file.write(ui->textEdit->toPlainText().toUtf8());
        file.close();
    }
}

void MainWindow::on_save_as_triggered()
{
    QString file_name = QFileDialog::getSaveFileName(this, QString ("Открыть файл"), QString(), QString("xml (*.xml);; all (*.*)"));
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
