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
#include <QSettings>
#include <boost/make_shared.hpp>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    service_(),
    work_(new boost::asio::io_service::work(service_)),
    serviceThread_(boost::bind(&boost::asio::io_service::run, &service_))
{
    qRegisterMetaType<alpha::protort::protocol::Packet_Payload>();
    qRegisterMetaType<alpha::protort::protocol::deploy::StatusResponse>();
    qRegisterMetaType<boost::system::error_code>();

    ui->setupUi(this);

    createConfigurationToolBar();

    load_session();
}

MainWindow::~MainWindow()
{
    for (int i = 0; i < ui->tabWidget->count(); ++i)
        if (!QFile(document(i)->filePath()).exists())
        {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this,
                                          tr("Выход"),
                                          tr("Файл %1 не сохранён. Сохранить?").arg(ui->tabWidget->tabText(i)),
                                          QMessageBox::Yes|QMessageBox::No);

            if (reply == QMessageBox::Yes)
            {
                saveDocument(i);
            }
        }
    save_session();
    work_.reset();
    service_.stop();
    if (serviceThread_.joinable())
        serviceThread_.join();
    delete ui;
}

void MainWindow::save_session()
{
    QSettings session("terminal.conf", QSettings::IniFormat);
    session.beginWriteArray("files");
    for (int i = 0; i < ui->tabWidget->count(); ++i)
    {
        session.setArrayIndex(i);
        session.setValue("filePath", document(i)->filePath());
    }
    session.endArray();
}

void MainWindow::load_session()
{
    QSettings session("terminal.conf", QSettings::IniFormat);
    int tabs_count = session.beginReadArray("files");
    for (int i = 0; i < tabs_count; ++i)
    {
        session.setArrayIndex(i);
        QString fileName = session.value("filePath").toString();
        load_file(fileName);
    }
    session.endArray();
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

void MainWindow::load_file(const QString& fileName)
{
    if (fileName.isEmpty())
        return;

    if (!QFile(fileName).exists())
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
    doc->setFilePath(fileName);
    addDocument(doc);
    setIcon(doc);
    addConfig(doc);
}

void MainWindow::on_load_file_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Открыть файл"), QString(),
                                                    tr("Описание приложения/Схема развёртывания (*.xml);; Все типы (*)"));
    load_file(fileName);
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
    QString name = doc->fileName();

    if(doc->isApp() && (m_apps->findText(name) == -1))
        m_apps->addItem(name, QVariant::fromValue(doc));

    if(doc->isDeploy() && (m_deploys->findText(name) == -1))
        m_deploys->addItem(name, QVariant::fromValue(doc));
}

void MainWindow::on_config_triggered()
{
    ConfigDialog dlg(this);
    for (int i = 0; i < ui->tabWidget->count(); ++i)
    {
        auto doc = document(i);
        if (!doc)
            continue;

        if (doc->isApp())
            dlg.loadApp(doc);
        else if (doc->isDeploy())
            dlg.loadDeploy(doc);
    }

    if (dlg.exec() && dlg.ready())
    {
        m_app = dlg.app();
        m_deploySchema = dlg.deploySchema();
        setCurrentDocument(m_apps, m_app);
        setCurrentDocument(m_deploys, m_deploySchema);
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

void MainWindow::onDeployConfigRequestFinished(const alpha::protort::protocol::deploy::Packet& packet)
{
    auto node = qobject_cast<RemoteNode *>(sender());
    writeLog(
                packet.has_error() ?
                    tr("Ошибка развертывания конфигурации на узел %1").arg(node->info()) :
                    tr("Конфигурация успешно развернута на узел %1").arg(node->info())
                    );
}

void MainWindow::onStatusRequestFinished(const alpha::protort::protocol::deploy::Packet& packet)
{
    auto node = qobject_cast<RemoteNode *>(sender());

    if (packet.has_error())
    {
        writeLog(tr("Ошибка получения статуса узла %1").arg(node->info()));
        return;
    }

    auto status = packet.response().status();

    writeStatusLog(tr("<Название узла - %1>").arg(QString::fromStdString(status.node_name())));
    writeStatusLog(tr("<Время работы - %2 сек.>").arg(QString::number(status.uptime())));
    writeStatusLog(tr("<Количество принятых пакетов - %3 (%4 байт)>")
                   .arg(QString::number(status.in_packets_count()))
                   .arg(QString::number(status.in_bytes_count())));
    writeStatusLog(tr("<Количество переданных пакетов - %3 (%4 байт)>")
                   .arg(QString::number(status.out_packets_count()))
                   .arg(QString::number(status.out_bytes_count())));

    writeStatusLog(tr("<Информация о компонентах>"));
    for (auto i = 0, size = status.component_statuses_size(); i < size; ++i)
    {
        auto component = status.component_statuses(i);

        writeStatusLog(tr("\t<Название компонента - %1>")
                       .arg(QString::fromStdString(component.name())));
        writeStatusLog(tr("\t\t<Количество принятых пакетов - %1>")
                       .arg(QString::number(component.in_packet_count())));
        writeStatusLog(tr("\t\t<Количество переданных пакетов - %1>")
                       .arg(QString::number(status.component_statuses(i).out_packet_count())));
    }

    writeStatusLog("\r\n");
}

void MainWindow::onStartRequestFinished(const alpha::protort::protocol::deploy::Packet& packet)
{
    auto node = qobject_cast<RemoteNode *>(sender());
    writeLog(
                packet.has_error() ?
                    tr("Ошибка запуска узла %1").arg(node->info()) :
                    tr("Узел %1 успешно запущен").arg(node->info())
                    );
}

void MainWindow::onStopRequestFinished(const alpha::protort::protocol::deploy::Packet& packet)
{
    auto node = qobject_cast<RemoteNode *>(sender());
    writeLog(
                packet.has_error() ?
                    tr("Ошибка останова узла %1").arg(node->info()) :
                    tr("Узел %1 успешно остановлен").arg(node->info())
                    );
}

void MainWindow::onConnected()
{
    auto node = qobject_cast<RemoteNode *>(sender());
    writeLog(tr("Успешное подключение к узлу %1").arg(node->info()));
}

void MainWindow::onConnectionFailed(const boost::system::error_code& err)
{
    auto node = qobject_cast<RemoteNode *>(sender());
    writeLog(tr("Невозможно подключиться к %1: %2")
             .arg(node->info())
             .arg(QString::fromStdString(err.message())));
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
        config_.parse_app(m_app->filePath().toStdString());
        config_.parse_deploy(m_deploySchema->filePath().toStdString());

        deploy_config_.parse_deploy(config_);
    }

    for (auto &remoteNode : remoteNodes_)
        remoteNode->shutdown();
    remoteNodes_.clear();

    for (auto node : deploy_config_.map_node)
    {
        auto remoteNode = boost::make_shared<RemoteNode>(node.second);
        remoteNodes_.append(remoteNode);

        connectRemoteNodeSignals(remoteNode.get());

        remoteNode->init(service_);
    }
}

void MainWindow::connectRemoteNodeSignals(RemoteNode *node)
{
    connect(node, &RemoteNode::connected, this, &MainWindow::onConnected);
    connect(node, &RemoteNode::connectionFailed, this, &MainWindow::onConnectionFailed);
    connect(node, &RemoteNode::deployConfigRequestFinished, this, &MainWindow::onDeployConfigRequestFinished);
    connect(node, &RemoteNode::statusRequestFinished, this, &MainWindow::onStatusRequestFinished);
    connect(node, &RemoteNode::startRequestFinished, this, &MainWindow::onStartRequestFinished);
    connect(node, &RemoteNode::stopRequestFinished, this, &MainWindow::onStopRequestFinished);
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

void MainWindow::createConfigurationToolBar()
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
    m_app = currentDocument(m_apps);
    m_deploySchema = currentDocument(m_deploys);
}

void MainWindow::activateDeploy() const
{
    if(!ui->deploy->isEnabled() && m_deploys->count() && m_apps->count())
        ui->deploy->setEnabled(true);
}

void MainWindow::button_clickedSetup()
{
    if (m_apps->count()> 0 && m_deploys->count() > 0) {
        setupConfigMembers();
        setActiveConfig();
        //activateDeploy();
        createRemoteNodes();
    }
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

void MainWindow::writeLog(const QString &message)
{
    ui->deployLog->append(message);
}

void MainWindow::writeStatusLog(const QString &message)
{
    ui->statusLog->append(message);
}

Document *MainWindow::currentDocument(QComboBox *combobox)
{
    return qobject_cast<Document *>(qvariant_cast<QTextEdit *>(combobox->currentData()));
}

void MainWindow::setCurrentDocument(QComboBox * combobox, Document *doc)
{
    combobox->setCurrentIndex(combobox->findData(QVariant::fromValue(doc)));
}
