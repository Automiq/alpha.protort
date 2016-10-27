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

void MainWindow::on_load_scheme_triggered()
{
    QFile deploy("deploy.xml");
    if (deploy.open(QIODevice::ReadWrite))
    {
        QByteArray deploy_text = deploy.readAll();
        ui->textEdit->setText(deploy_text);
        deploy.close();
    }
    else
        ui->textEdit->setText("ERROR! Cannot open deploy.xml");
}

void MainWindow::on_load_descripton_triggered()
{
    QFile app("app.xml");
    if (app.open(QIODevice::ReadWrite))
    {
        QByteArray app_text = app.readAll();
        ui->textEdit->setText(app_text);
        app.close();
    }
    else
        ui->textEdit->setText("ERROR! Cannot open app.xml");
}

