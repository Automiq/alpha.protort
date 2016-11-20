#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "client.h"
#include "server.h"
#include "connection.h"
#include "parser.h"
#include "configdialog.h"

#include <QComboBox>
#include <QIcon>
#include <QLabel>
#include <QFileDialog>
#include <QTextEdit>
#include <QTextStream>
#include <QToolBar>
#include <QObject>
#include <QPushButton>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setupWindowConfigurations();
}

MainWindow::~MainWindow()
{
    delete ui;
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
    setIcon(doc);
    defineToAddConf(doc);
}

void MainWindow::defineToAddConf(Document *doc)
{
    QString name = doc->fileName();

    if(doc->isApp() && m_apps->findText(name) == -1)
        addConfig(name, m_apps);

    if(doc->isDeploy() && m_deploys->findText(name) == -1)
        addConfig(name, m_deploys);
}

void MainWindow::deleteConfig(QComboBox *ptr, QString &nameD)
{
    int indx = ptr->findText(nameD);
    if(indx != -1)
        ptr->removeItem(indx);
}

void MainWindow::delConfig(Document *doc)
{
    QString name = doc->fileName();

    if(doc->isApp())
        deleteConfig(m_app, name);

    if(doc->isDeploy())
        deleteConfig(m_deploy, name);
}

void MainWindow::updateConfig(Document *doc, Document::Kind before, Document::Kind after)
{
    if(after == before)
        return;

    if(before == Document::Kind::Unknown && after != Document::Kind::Unknown)
        defineToAddConf(doc);

    QString name = doc->fileName();
    if(before == Document::Kind::App && after == Document::Kind::Deploy)
    {
        deleteApp(name);
        addConfig(name, m_deploys);
    }
    if(before == Document::Kind::Deploy && after == Document::Kind::App)
    {
        deleteDeploy(name);
        addConfig(name, m_apps);
    }
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

void MainWindow::addConfig(QString &name, QComboBox *ptr)
{
    ptr->addItem(name);
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

        if(text_edit->isApp())
            dlg.loadApp(nname);
        if(text_edit->isDeploy())
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

void MainWindow::setTabName(int index, const QString &name)
{
    ui->tabWidget->setTabText(index, QString(QFileInfo(name).fileName()));
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

void MainWindow::on_close_file_triggered()
{
    close_tab(ui->tabWidget->currentIndex());
}

void MainWindow::close_tab(int index)
{

    auto doc = dynamic_cast<Document*> (ui->tabWidget->widget(index));
    delConfig(doc);
    ui->tabWidget->widget(index)->deleteLater();
    ui->tabWidget->removeTab(index);
}

void MainWindow::on_status_request_triggered()
{
    ui->text_browser_status->clear();
    for (int i = 0; i < m_statOut.size(); ++i)
    {
        ui->text_browser_status->insertPlainText("<Название узла - " +
                                                 QString::fromStdString(m_statOut[i].node_name())
                                                 + ">\n<Время работы - " +
                                                 QString::number(m_statOut[i].uptime()) + ">\n<Количество принятых пакетов - "
                                                 + QString::number(m_statOut[i].in_packets_count()) +
                                                 " (" + QString::number(m_statOut[i].in_bytes_count())
                                                 + " б)" + ">\n<Количество переданных пакетов - "
                                                 + QString::number(m_statOut[i].out_packets_count()) + " ("
                                                 + QString::number(m_statOut[i].out_bytes_count())
                                                 + " б)"+ ">\n\n<Информация о компонентах>\n\n");
        for (int j = 0; j < m_statOut[i].component_statuses_size(); ++j)
        {
            ui->text_browser_status->insertPlainText("<Название компонента - " +
                                                     QString::fromStdString(m_statOut[i].component_statuses(i).name()) +
                                                     "\n<Количество принятых пакетов - >"
                                                     + QString::number(m_statOut[i].component_statuses(i).in_packet_count()) +
                                                     ">\n<Количество переданных пакетов - >" +
                                                     QString::number(m_statOut[i].component_statuses(i).out_packet_count()) +
                                                     "\n\n");
        }
        ui->text_browser_status->insertPlainText("\n\n");
    }
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

void MainWindow::saveDocument(int index)
{
    if(index == -1)
        return;

    auto doc = dynamic_cast<Document*> (ui->tabWidget->widget(index));

    if(!doc)
        return;

    Document::Kind kindBeforeSave = doc->kind();

    if(!doc->save())
        return;

    Document::Kind kindAfterSave = doc->kind();

    updateConfig(doc, kindBeforeSave, kindAfterSave);

    QString nfname = doc->fileName();

    if(nfname != ui->tabWidget->tabText(index))
        setTabName(index, nfname);
    setIcon(doc);
}

void MainWindow::addDocument(Document *doc)
{
    int index = ui->tabWidget->indexOf(doc);
    if (index != -1)
        return;
    ui->tabWidget->addTab(doc, fixedWindowTitle(doc));
    ui->tabWidget->setTabEnabled(index, true);
    ui->tabWidget->setCurrentIndex(ui->tabWidget->count()-1);
}

void MainWindow::setIcon(Document *doc)
{
    if(doc->isApp())
    {
        ui->tabWidget->setTabIcon(ui->tabWidget->indexOf(doc), QIcon(":/images/pen.png"));
        return;
    }

    if(doc->isDeploy())
    {
        ui->tabWidget->setTabIcon(ui->tabWidget->indexOf(doc), QIcon(":/images/cog.png"));
        return;
    }

    ui->tabWidget->setTabIcon(ui->tabWidget->indexOf(doc), QIcon(":/images/file.png"));
}

void MainWindow::setupWindowConfigurations()
{
    m_deploys = new QComboBox();
    m_apps = new QComboBox();

    QLabel *app = new QLabel("Описание: ");
    ui->mainToolBar->addWidget(app);
    ui->mainToolBar->addWidget(m_apps);

    QLabel *sсhema = new QLabel("Схема: ");
    ui->mainToolBar->addWidget(sсhema);
    ui->mainToolBar->addWidget(m_deploys);

    QPushButton *setupConfig = new QPushButton();
    setupConfig->setText("Установить");
    ui->mainToolBar->addWidget(setupConfig);
}
