#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "client.h"
#include "server.h"
#include "connection.h"
#include "parser.h"

#include <QFileDialog>
#include <QTextEdit>
#include <QObject>
#include <QTextStream>

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

void MainWindow::close_tab(int index)
{
    ui->tabWidget->widget(index)->deleteLater();
    ui->tabWidget->removeTab(index);
    if (ui->tabWidget->count() == 0)
        close();
}

void MainWindow::setTabName(int index, const QString &name)
{
    ui->tabWidget->setTabText(index, QString(QFileInfo(name).fileName()));
}

void MainWindow::saveDocument(int index)
{
    if(index == -1)
        return;

    auto doc = dynamic_cast<Document*> (ui->tabWidget->widget(index));

    if(!doc)
        return;

    if(!doc->save())
        return;

    QString nfname = doc->fileName();
    if(nfname != ui->tabWidget->tabText(index))
        setTabName(index, nfname);

}

void MainWindow::on_save_file_triggered()
{
    saveDocument(ui->tabWidget->currentIndex());
}

void MainWindow::on_save_all_triggered()
{
    for (int i = 0; i < ui->tabWidget->count(); ++i)
        saveDocument(i);
}

void MainWindow::on_exit_triggered()
{
    ui->stop->triggered(true);
    close();
}

void MainWindow::on_load_file_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Открыть файл"), QString(),
                                                    tr("Описание приложения/Схема развёртывания (*.xml);; Все типы (*)"));

    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(this,
                             tr("Ошибка"),
                             tr("Ошибка открытия файла\n%1").arg(fileName));
        return;
    }

    Document *doc = new Document();
    doc->setText(file.readAll());
    doc->setFileName(fileName);
    addDocument(doc);
}

void MainWindow::addDocument(Document *doc)
{
    if (ui->tabWidget->indexOf(doc) != -1)
        return;
    ui->tabWidget->addTab(doc, fixedWindowTitle(doc));
}

QString MainWindow::fixedWindowTitle(const Document *doc) const
{
    QString title = doc->fileName();

    if (title.isEmpty())
        title = tr("Безымянный");
    else
        title = QFileInfo(title).fileName();

    QString result;

    for (int i = 0; ; ++i)
    {
        result = title;
        if (i > 0)
            result += QString::number(i);

        bool unique = true;
        for (int j = 0; j < ui->tabWidget->count(); ++j)
        {
            const QWidget *widget = ui->tabWidget->widget(j);
            if (widget == doc)
                continue;
            if (result == ui->tabWidget->tabText(j))
            {
                unique = false;
                break;
            }
        }

        if (unique)
            break;
    }
    return result;
}


void MainWindow::on_create_file_triggered()
{
    Document *tmp = new Document();
    addDocument(tmp);
}

void MainWindow::on_tabWidget_tabCloseRequested(int index)
{
    close_tab(index);
}

void MainWindow::on_config_triggered()
{
    ConfigDialog dlg(this);
    for (int i = 0; i < ui->tabWidget->count(); ++i)
    {
        auto text_edit = dynamic_cast<Document*> (ui->tabWidget->widget(i));
        if (!text_edit)
            continue;

        QString nname = text_edit->fileName();

        if(text_edit->Kind::App == text_edit->kind())
            dlg.loadApp(nname);
        if(text_edit->Kind::Deploy == text_edit->kind())
            dlg.loadDeploy(nname);
    }

    if (dlg.exec())
    {
        m_app = dlg.app();
        m_deploySchema = dlg.deploySchema();
        ui->start->setDisabled(true);
        ui->stop->setDisabled(true);
        ui->deploy->setEnabled(true);
    }

    ui->textBrowser->setText("Загрузка описания приложения...\n" + m_app +
                             "\n" +"Описание загружено" + "\n" +
                             "Загрузка схемы развёртывания..." +
                             "\n" + m_deploySchema +
                             "\n" + "Схема загружена" +
                             "\n" + "Упс! Не могу развернуть." +
                             "\n Требуются доработки.");
}

void MainWindow::on_close_file_triggered()
{
    close_tab(ui->tabWidget->currentIndex());
}

void MainWindow::on_start_triggered()
{
    ui->start->setDisabled(true);
    ui->status_request->setEnabled(true);
    ui->stop->setEnabled(true);
}

void MainWindow::on_stop_triggered()
{
    ui->start->setEnabled(true);
    ui->status_request->setDisabled(true);
    ui->stop->setDisabled(true);
}

void MainWindow::on_deploy_triggered()
{
    ui->deploy->setDisabled(true);
    ui->start->setEnabled(true);
}

void MainWindow::on_status_request_triggered()
{
    ui->text_browser_status->clear();
    for (int i = 0; i < stat_out.size(); ++i)
    {
        ui->text_browser_status->insertPlainText("<Название узла - " +
                                                 QString::fromStdString(stat_out[i].node_name())
                                                 + ">\n<Время работы - " +
                                                 QString::number(stat_out[i].uptime()) + ">\n<Количество принятых пакетов - "
                                                 + QString::number(stat_out[i].in_packets_count()) +
                                                 " (" + QString::number(stat_out[i].in_bytes_count())
                                                 + " б)" + ">\n<Количество переданных пакетов - "
                                                 + QString::number(stat_out[i].out_packets_count()) + " ("
                                                 + QString::number(stat_out[i].out_bytes_count())
                                                 + " б)"+ ">\n\n<Информация о компонентах>\n\n");
        for (int j = 0; j < stat_out[i].component_statuses_size(); ++j)
        {
            ui->text_browser_status->insertPlainText("<Название компонента - " +
                                                     QString::fromStdString(stat_out[i].component_statuses(i).name()) +
                                                     "\n<Количество принятых пакетов - >"
                                                     + QString::number(stat_out[i].component_statuses(i).in_packet_count()) +
                                                     ">\n<Количество переданных пакетов - >" +
                                                     QString::number(stat_out[i].component_statuses(i).out_packet_count()) +
                                                     "\n\n");
        }
        ui->text_browser_status->insertPlainText("\n\n");
    }
}
