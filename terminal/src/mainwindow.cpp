#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QTextEdit>
#include <QObject>

using namespace alpha::protort::protocol::deploy;

StatusResponse stat_out;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    openFiles = new QString();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::close_tab(int index)
{
    ui->tabWidget->currentWidget()->deleteLater();
    ui->tabWidget->removeTab(index);
    if (ui->tabWidget->count() == 0)
        close();
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
    if(!openFiles->isEmpty())
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
}

void MainWindow::on_exit_triggered()
{
    ui->stop->triggered(true);
    close();
}

void MainWindow::on_load_file_triggered()
{
    QString file_name = QFileDialog::getOpenFileName(this, QString ("Открыть файл"), QString(), QString("xml (*.xml);; all (*)"));
    openFiles->push_back(file_name);
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
    openFiles->push_back(file_name);
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
    close_tab(index);
}

QTextEdit* MainWindow::createNewTab(const QString &name)
{
    QTextEdit *text_edit = new QTextEdit();
    ui->tabWidget->setCurrentIndex(ui->tabWidget->addTab(text_edit, name));
    ui->tabWidget->setTabText(ui->tabWidget->currentIndex(), name);
    return text_edit;
}

void MainWindow::on_config_triggered()
{
    ConfigDialog dlg(this);
    if (dlg.exec())
    {
        m_app = dlg.app();
        m_deploySchema = dlg.deploySchema();
        ui->start->setDisabled(true);
        ui->stop->setDisabled(true);
        ui->deploy->setEnabled(true);
    }
}

void MainWindow::on_start_triggered()
{
    ui->start->setDisabled(true);
    ui->stop->setEnabled(true);
}

void MainWindow::on_stop_triggered()
{
    ui->start->setEnabled(true);
    ui->stop->setDisabled(true);
}

void MainWindow::on_deploy_triggered()
{
    ui->deploy->setDisabled(true);
    ui->start->setEnabled(true);
}

void MainWindow::on_close_file_triggered()
{
    close_tab(ui->tabWidget->currentIndex());
}

void MainWindow::on_Status_request_triggered()
{
    ui->text_browser_status->clear();
    ui->text_browser_status->insertPlainText("<Название узла - " + QString::fromStdString(stat_out.node_name()) + ">\n");
    ui->text_browser_status->insertPlainText("<Количество принятых пакетов - " + QString::number(stat_out.counter_in_packets()) +
                                             " (" + QString::number(stat_out.counter_in_bytes()/1000) + " кб)" + ">\n");
    ui->text_browser_status->insertPlainText("<Количество переданных пакетов - " + QString::number(stat_out.counter_out_packets()) + " ("
                                             + QString::number(stat_out.counter_out_bytes()/1000) + " кб)"+ ">\n");
    ui->text_browser_status->insertPlainText("<Информация о компонентах>\n");
    for (int i = 0; i < stat_out.component_status_list_size(); i++)
    {
        ui->text_browser_status->insertPlainText("<Название компонента - " + QString::fromStdString(stat_out.component_status_list(i).name()) + ">\n");
        ui->text_browser_status->insertPlainText("<Количество принятых пакетов - " + QString::number(stat_out.component_status_list(i).counter_in_packets()) + ">\n");
        ui->text_browser_status->insertPlainText("<Количетво переданных пакетов - " + QString::number(stat_out.component_status_list(i).counter_out_packets()) + ">\n\n");

    }
}
