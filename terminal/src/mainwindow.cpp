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
#include <boost/make_shared.hpp>

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    service_(),
    work_(new boost::asio::io_service::work(service_)),
    protoThread_(boost::bind(&boost::asio::io_service::run, &service_))
{
    qRegisterMetaType<alpha::protort::protocol::Packet_Payload>();
    qRegisterMetaType<alpha::protort::protocol::deploy::StatusResponse>();
    qRegisterMetaType<boost::system::error_code>();
    ui->setupUi(this);
    setupWindowConfigurations();
}

MainWindow::~MainWindow()
{
    work_.reset();
    service_.stop();
    if (protoThread_.joinable())
        protoThread_.join();
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
    addConfig(doc);
}

void MainWindow::deleteConfig(QComboBox *ptr, const QString &name)
{
    int indx = ptr->findText(QFileInfo(name).fileName());
    if(indx != -1)
        ptr->removeItem(indx);
}

void MainWindow::delConfig(Document *doc)
{
    QString name = doc->filePath();

    deleteConfig(m_apps, name);
    deleteConfig(m_deploys, name);
}

void MainWindow::updateConfig(Document *doc)
{
    delConfig(doc);
    addConfig(doc);
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

void MainWindow::addConfig(Document *doc)
{
    QString name = doc->filePath();

    if(doc->isApp() && (m_apps->findText(name) == -1))
        m_apps->addItem(QFileInfo(name).fileName());

    if(doc->isDeploy() && (m_deploys->findText(name) == -1))
        m_deploys->addItem(QFileInfo(name).fileName());
}

void MainWindow::on_config_triggered()
{
    ConfigDialog dlg(this);
    for (int i = 0; i < ui->tabWidget->count(); ++i)
    {
        auto doc = document(i);
        if (!doc)
            continue;

        QString nname = doc->filePath();

        if (doc->isApp())
            dlg.loadApp(nname);
        else if (doc->isDeploy())
            dlg.loadDeploy(nname);
    }

    if (dlg.exec())
    {
        m_app = dlg.app();
        m_deploySchema = dlg.deploySchema();
        m_apps->setCurrentIndex(m_apps->findText(m_app));
        m_deploys->setCurrentIndex(m_deploys->findText(m_deploySchema));
        setActiveConfig();
        activateDeploy();
    }
}

void MainWindow::resetDeployActions() const
{
    ui->stop->setDisabled(true);
    ui->start->setEnabled(true);
}

void MainWindow::showLog() const
{
}

void MainWindow::onDeployConfigRequestFinished()
{
    auto remoteNode = qobject_cast<RemoteNode *>(sender());
    ui->textBrowser->insertPlainText(
                "Configuration on " +
                QString::fromStdString(remoteNode->node_information_.name) +
                " has been connected.\n");
}

void MainWindow::onstatusRequestFinished(alpha::protort::protocol::deploy::Packet const& status)
{
    ui->text_browser_status->insertPlainText("<Название узла - " +
                                             QString::fromStdString(status.response().status().node_name())
                                             + ">\n<Время работы - " +
                                             QString::number(status.response().status().uptime()) + " сек." + ">\n<Количество принятых пакетов - "
                                             + QString::number(status.response().status().in_packets_count()) +
                                             " (" + QString::number(status.response().status().in_bytes_count())
                                             + " байт)" + ">\n<Количество переданных пакетов - "
                                             + QString::number(status.response().status().out_packets_count()) + " ("
                                             + QString::number(status.response().status().out_bytes_count())
                                             + " байт)"+ ">\n\n<Информация о компонентах>\n\n");
    for (int j = 0; j < status.response().status().component_statuses_size(); ++j)
    {
        ui->text_browser_status->insertPlainText("<Название компонента - " +
                                                 QString::fromStdString(status.response().status().component_statuses(j).name()) +
                                                 ">\n<Количество принятых пакетов - "
                                                 + QString::number(status.response().status().component_statuses(j).in_packet_count()) +
                                                 ">\n<Количество переданных пакетов - " +
                                                 QString::number(status.response().status().component_statuses(j).out_packet_count()) +
                                                 ">\n\n");
    }
    ui->text_browser_status->insertPlainText("\n\n");
}

void MainWindow::onstartRequestFinished()
{
    auto remoteNode = qobject_cast<RemoteNode *>(sender());
    ui->textBrowser->insertPlainText(
                "Configuration on " +
                QString::fromStdString(remoteNode->node_information_.name) +
                " has been started.\n");
}

void MainWindow::onstopRequestFinished()
{
    auto remoteNode = qobject_cast<RemoteNode *>(sender());
    ui->textBrowser->insertPlainText(
                "Configuration on " +
                QString::fromStdString(remoteNode->node_information_.name) +
                " has been stopped.\n");
}

void MainWindow::onconnected()
{
    auto remoteNode = qobject_cast<RemoteNode *>(sender());
    ui->textBrowser->insertPlainText(
                "Node " +
                QString::fromStdString(remoteNode->node_information_.name) +
                " has been connected.\n");
}

void MainWindow::onconnectionFailed(const boost::system::error_code &)
{
    auto remoteNode = qobject_cast<RemoteNode *>(sender());
    ui->textBrowser->insertPlainText(
                "Node " +
                QString::fromStdString(remoteNode->node_information_.name) +
                " can not be connected.\n");
}

void MainWindow::setTabName(int index, const QString &name)
{
    ui->tabWidget->setTabText(index, QString(QFileInfo(name).fileName()));
}

void MainWindow::on_start_triggered()
{
    ui->deploy->setDisabled(true);
    ui->start->setDisabled(true);
    ui->stop->setEnabled(true);

    alpha::protort::protocol::Packet_Payload payload;
    payload.mutable_deploy_packet()->set_kind(alpha::protort::protocol::deploy::Start);

    for (auto &remoteNode: remoteNodes_)
        remoteNode->async_start(payload);
}

void MainWindow::on_stop_triggered()
{
    if(! ui->deploy->isEnabled())
        ui->start->setEnabled(true);
    ui->stop->setDisabled(true);

    alpha::protort::protocol::Packet_Payload payload;
    payload.mutable_deploy_packet()->set_kind(alpha::protort::protocol::deploy::Stop);

    for (auto &remoteNode: remoteNodes_)
        remoteNode->async_stop(payload);
}

void MainWindow::showMessage()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this,
                                  "Смена конфигурации",
                                  "Старая конфигурация будет остановлена и загружена новая, вы уверены?",
                                QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes)
    {
        deploy();
    }
}

void MainWindow::deploy()
{
    resetDeployActions();
    ui->deploy->setDisabled(true);

    for (auto &remoteNode: remoteNodes_)
        remoteNode->async_deploy(deploy_config_);
}

void MainWindow::on_deploy_triggered()
{
    if(ui->start->isEnabled() || ui->stop->isEnabled())
        showMessage();
    else
        deploy();
}

void MainWindow::on_close_file_triggered()
{
    close_tab(ui->tabWidget->currentIndex());
}

void MainWindow::close_tab(int index)
{
    auto doc = document(index);
    delConfig(doc);
    ui->tabWidget->widget(index)->deleteLater();
    ui->tabWidget->removeTab(index);
}

void MainWindow::on_status_request_triggered()
{
    ui->text_browser_status->clear();

    alpha::protort::protocol::Packet_Payload status;
    status.mutable_deploy_packet()->set_kind(alpha::protort::protocol::deploy::GetStatus);

    for (auto &remoteNode: remoteNodes_)
        remoteNode->async_status(status);
}

QString MainWindow::fixedWindowTitle(const Document *doc) const
{
    QString title = doc->filePath();

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

void MainWindow::createRemoteNodes()
{
    {
        alpha::protort::parser::configuration config_;
        config_.parse_app(m_app.toStdString());
        config_.parse_deploy(m_deploySchema.toStdString());

        deploy_config_.parse_deploy(config_);
    }

    for (auto &remoteNode : remoteNodes_)
        remoteNode->shutdown();

    if(remoteNodes_.size() > 0)
        remoteNodes_.clear();

    for (auto node : deploy_config_.map_node)
    {
        auto remoteNode = boost::make_shared<RemoteNode>(node.second);

        connect(remoteNode.get(),
                &RemoteNode::connected,
                this,
                &MainWindow::onconnected);

        connect(remoteNode.get(),
                &RemoteNode::connectionFailed,
                this,
                &MainWindow::onconnectionFailed);

        connect(remoteNode.get(),
                &RemoteNode::deployConfigRequestFinished,
                this,
                &MainWindow::onDeployConfigRequestFinished);

        connect(remoteNode.get(),
                &RemoteNode::statusRequestFinished,
                this,
                &MainWindow::onstatusRequestFinished);

        connect(remoteNode.get(),
                &RemoteNode::startRequestFinished,
                this,
                &MainWindow::onstartRequestFinished);

        connect(remoteNode.get(),
                &RemoteNode::stopRequestFinished,
                this,
                &MainWindow::onstopRequestFinished);

        remoteNode->init(service_);

        remoteNodes_.append(remoteNode);
    }
}

void MainWindow::saveDocument(int index)
{
    if(index == -1)
        return;

    auto doc = document(index);

    if(!doc)
        return;

    if(!doc->save())
        return;

    updateConfig(doc);

    QString nfname = doc->filePath();

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
    ui->tabWidget->setCurrentWidget(doc);
    setIcon(doc);
}

void MainWindow::setTabIco(Document *doc, const QString &srcPath) const
{
    ui->tabWidget->setTabIcon(ui->tabWidget->indexOf(doc), QIcon(srcPath));
}

void MainWindow::setIcon(Document *doc)
{
    switch(doc->kind())
    {
    case Document::Kind::App:
        setTabIco(doc, ":/images/pen.png");
        break;
    case Document::Kind::Deploy:
        setTabIco(doc,":/images/cog.png");
        break;
    default:
        setTabIco(doc,":/images/file.png");
        break;
    }
}

void MainWindow::addWidgetOnBar(QWidget* newWidget) const
{
    ui->mainToolBar->addWidget(newWidget);
}

void MainWindow::setupWindowConfigurations()
{
    m_deploys = new QComboBox();
    m_apps = new QComboBox();

    QLabel *app = new QLabel(tr("Описание: "));
    addWidgetOnBar(app);
    addWidgetOnBar(m_apps);

    QLabel *schema = new QLabel(tr("Схема: "));
    addWidgetOnBar(schema);
    addWidgetOnBar(m_deploys);

    m_setupConfig = new QPushButton();
    m_setupConfig->setText(tr("Установить"));
    addWidgetOnBar(m_setupConfig);

    connect(m_setupConfig, SIGNAL(clicked()), this, SLOT(button_clickedSetup()));
}

void MainWindow::setupConfigMembers()
{
    m_app = m_apps->currentText();
    m_deploySchema = m_deploys->currentText();
}

void MainWindow::activateDeploy() const
{
    if(!ui->deploy->isEnabled() && m_deploys->count() && m_apps->count())
        ui->deploy->setEnabled(true);
}

void MainWindow::button_clickedSetup()
{
    setupConfigMembers();
    setActiveConfig();
    //activateDeploy();
    createRemoteNodes();
}

void MainWindow::setActiveConfig()
{
    for (int i = 0; i < ui->tabWidget->count(); ++i)
    {
        setupActiveIcon(i);
    }
}

void MainWindow::setupActiveIcon(const int &index)
{
    auto doc = document(index);
    QString nameApp = QFileInfo(m_apps->currentText()).fileName();
    QString nameDeploy = QFileInfo(m_deploys->currentText()).fileName();

    setIcon(doc);
    if((QFileInfo(doc->filePath()).fileName() == nameApp) && (doc->isApp()))
        setTabIco(doc,":/images/greenPen.png");

    if((QFileInfo(doc->filePath()).fileName() == nameDeploy) && (doc->isDeploy()))
        setTabIco(doc,":/images/greenCog.png");
}

Document* MainWindow::document(int index)
{
    return dynamic_cast<Document*> (ui->tabWidget->widget(index));
}
