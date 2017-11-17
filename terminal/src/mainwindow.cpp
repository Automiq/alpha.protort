#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "client.h"
#include "server.h"
#include "connection.h"
#include "parser.h"
#include "configdialog.h"

#include <QPushButton>
#include <QAbstractButton>
#include <QComboBox>
#include <QIcon>
#include <QLabel>
#include <QFileDialog>
#include <QTextEdit>
#include <QTextStream>
#include <QToolBar>
#include <QObject>
#include <QSettings>
#include <QXmlStreamReader>
#include <QWidget>
#include <QTimer>
#include <QHBoxLayout>


#include <QToolTip>
#include <boost/make_shared.hpp>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_statusTimer(new QTimer(this)),
    service_(),
    work_(new boost::asio::io_service::work(service_)),
    serviceThread_(boost::bind(&boost::asio::io_service::run, &service_))
{
    qRegisterMetaType<alpha::protort::protocol::Packet_Payload>();
    qRegisterMetaType<alpha::protort::protocol::deploy::Packet>();
    qRegisterMetaType<alpha::protort::protocol::deploy::StatusResponse>();
    qRegisterMetaType<boost::system::error_code>();

    ui->setupUi(this);

    createConfigurationToolBar();

    load_session();

    connect(m_statusTimer, &QTimer::timeout, this, &MainWindow::on_status_triggered);

    TreeModel *model = new TreeModel(remoteNodes_);
    ui->treeStatus->setModel(model);
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
    addConfig(doc);
    addDocument(doc);
    setIcon(doc);

}

void MainWindow::on_load_file_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Открыть файл"), QString(),
                                                    tr("Описание приложения/Схема развёртывания (*.xml);; Все типы (*)"));
    load_file(fileName);
}

void MainWindow::delDocFromComboBox(QComboBox* combobox, Document* doc)
{
    combobox->removeItem(combobox->findData(QVariant::fromValue(doc)));
}

void MainWindow::delConfig(Document *doc)
{
    delDocFromComboBox(m_apps, doc);
    delDocFromComboBox(m_deploys, doc);
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
    if(doc->isApp() && (m_apps->findData(QVariant::fromValue(doc)) == -1)){
        m_apps->addItem(fixedWindowTitle(doc), QVariant::fromValue(doc));
        setComboBoxToolTip(m_apps, doc);
    }

    if(doc->isDeploy() && (m_deploys->findData(QVariant::fromValue(doc)) == -1)){
        m_deploys->addItem(fixedWindowTitle(doc), QVariant::fromValue(doc));
        setComboBoxToolTip(m_deploys, doc);
    }
}

void MainWindow::setComboBoxToolTip(QComboBox *combobox, Document *doc)
{
     combobox->setItemData(combobox->findData(QVariant::fromValue(doc)), doc->filePath(), Qt::ToolTipRole);
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
    }
}

void MainWindow::resetDeployActions() const
{
    if (!ui->start->isEnabled() && !ui->stop->isEnabled())
        ui->start->setEnabled(true);
}

void MainWindow::showLog() const
{
}

void MainWindow::onDeployConfigRequestFinished(const alpha::protort::protocol::deploy::Packet& packet)
{
    deploying = false;
    auto node = qobject_cast<RemoteNode *>(sender());
    writeLog(
                packet.has_error() ?
                    tr("Ошибка развертывания конфигурации на узел %1").arg(node->info()) :
                    tr("Конфигурация успешно развернута на узел %1").arg(node->info())
                    );
}

/*!
 * \brief Вызывется по окончании процесса резервного перехода.
 * \param packet - пакет со статусом.
 */
void MainWindow::onBackupTransitionRequestFinished(const alpha::protort::protocol::deploy::Packet& packet)
{
    //Тут нужно перепривязать кнопку на компоненту нового мастера
    auto node = qobject_cast<RemoteNode *>(sender());
    //remoteNodes_.front()
    //remoteNodes_. Поиск функции, которая даст мне указатель на слудующую ноду.
    writeLog(
                packet.has_error() ?
                    tr("Ошибка резервного перехода на узеле %1").arg(node->info()) :
                    tr("Резервный переход успешно выполнен для узела %1").arg(node->info())
                    );
}

void MainWindow::onStatusRequestFinished(const alpha::protort::protocol::deploy::Packet& packet)
{
    auto node = qobject_cast<RemoteNode *>(sender());

    if(node->isConnected() && node->backupStatus() == 0 && !(node->bakupPushButtonStatus()))
    {
        TreeModel *mod = reinterpret_cast<TreeModel*>(ui->treeStatus->model());
        int row = mod->indexOfNode(node);//только если вытащим из private функцию.
        QModelIndex currentModelIndex = ui->treeStatus->model()->index( row, mod->BackupTransitionColumn());// Индекс ячейки перехода
        RemoteNodePtr remNode = remoteNodes_.at(row);


        QPushButton *bakupTransitionButton = new QPushButton;
        bakupTransitionButton->setIcon(QIcon(":/images/master.png"));
        bakupTransitionButton->setProperty("row", (QVariant)row);
        bakupTransitionButton->setFixedSize(40,20);
        bakupTransitionButton->setEnabled(true);

        ui->treeStatus->setIndexWidget(currentModelIndex, bakupTransitionButton);// Устанавливаем кнопку в соответстующую ячейку

        connect(bakupTransitionButton,SIGNAL(clicked()),this,SLOT(on_backup_transition()));
        bakupTransitionButton->installEventFilter(this);

        /////////////////////////////////////////////////////////////////
        remNode->setBackupPushButtonStatus(true);
    }
    if(node->isConnected() && node->backupStatus() == 1 && node->bakupPushButtonStatus())
    {
        TreeModel *mod = reinterpret_cast<TreeModel*>(ui->treeStatus->model());
        int row = mod->indexOfNode(node);//только если вытащим из private функцию.

        RemoteNodePtr masteRemoteNode = remoteNodes_.at(row),
                      slaveRemoteNode = remoteNodes_.at(row+1),
                      variable(masteRemoteNode);
        masteRemoteNode->setBackupPushButtonStatus(false);
        slaveRemoteNode->setBackupPushButtonStatus(true);
        masteRemoteNode = slaveRemoteNode;
        slaveRemoteNode = variable;


        on_status_triggered();
    }

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

        writeStatusLog(tr("<Название компонента - %1>")
                       .arg(QString::fromStdString(component.name())));
        writeStatusLog(tr("<Количество принятых пакетов - %1>")
                       .arg(QString::number(component.in_packet_count())));
        writeStatusLog(tr("<Количество переданных пакетов - %1>")
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
    activateDeploy();
    activateStatus();
}

void MainWindow::onConnectionFailed(const boost::system::error_code& err)
{
    auto node = qobject_cast<RemoteNode *>(sender());

//    if(node->backupStatus() == 0 && node->bakupPushButtonStatus())
//    {
//        TreeModel *mod = reinterpret_cast<TreeModel*>(ui->treeStatus->model());
//        int row = mod->indexOfNode(node);

//        QMessageBox *mbTest = new QMessageBox;
//        mbTest->setText(QString("%1").arg(QString::number(row,16)));// Проверка урвня и имени ноды
//        mbTest->show();

//        QModelIndex currentModelIndex = ui->treeStatus->model()->index( row, mod->BackupTransitionColumn());// Индекс ячейки перехода
//        RemoteNodePtr remoteNode = remoteNodes_.at(row);

//        QPushButton* pushButton = reinterpret_cast<QPushButton*>(ui->treeStatus->indexWidget(currentModelIndex));
//        delete pushButton;

//        remoteNode->setBackupPushButtonStatus(false);
//    }

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
    ui->start->setDisabled(true);
    ui->stop->setEnabled(true);

    alpha::protort::protocol::Packet_Payload payload;
    payload.mutable_deploy_packet()->set_kind(alpha::protort::protocol::deploy::Start);

    size_t i(0);
    for (auto &remoteNode: remoteNodes_)
    {
        remoteNode->async_start(payload);
        TreeModel *mod = reinterpret_cast<TreeModel*>(ui->treeStatus->model());
        QModelIndex currentModelIndex = ui->treeStatus->model()->index(i, mod->BackupTransitionColumn());//Индекс ячейки перехода
        if(remoteNode->backupStatus() == 0)
        {
            ui->treeStatus->indexWidget(currentModelIndex)->setEnabled(true);
        }
            ++i;
    }
}

void MainWindow::on_stop_triggered()
{
    ui->start->setEnabled(true);
    ui->stop->setDisabled(true);

    alpha::protort::protocol::Packet_Payload payload;
    payload.mutable_deploy_packet()->set_kind(alpha::protort::protocol::deploy::Stop);

    size_t i(0);
    for (auto &remoteNode: remoteNodes_)
    {
        remoteNode->async_stop(payload);
        TreeModel *mod = reinterpret_cast<TreeModel*>(ui->treeStatus->model());
        QModelIndex currentModelIndex = ui->treeStatus->model()->index(i, mod->BackupTransitionColumn());
        if(remoteNode->backupStatus() == 0)
        {
            ui->treeStatus->indexWidget(currentModelIndex)->setEnabled(false);
        }
        ++i;
    }
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
    ui->status->setEnabled(true);
    ui->deploy->setDisabled(true);

    for (auto &remoteNode: remoteNodes_)
    {
        remoteNode->async_deploy(deploy_config_);
    }
    TreeModel *model = new TreeModel(remoteNodes_);
    model->setupModelData(remoteNodes_);

    ui->treeStatus->show();
}

bool MainWindow::eventFilter(QObject *object, QEvent *event)
{
        QPushButton *pushButton = qobject_cast<QPushButton*>(object);
            if(event->type() == QEvent::Enter && pushButton->isEnabled())
            {
                pushButton->setIcon(QIcon(":/images/backupTransitionICO.png"));
                return true;
            }
            if(event->type() == QEvent::Leave && pushButton->isEnabled())
            {
                pushButton->setIcon(QIcon(":/images/master.png"));
                return true;
            }
//            if(event->type() == QEvent::MouseButtonPress && pushButton->isEnabled())
//            {

//                int row = pushButton->property("row").toInt();// У отправителя сигнала узнаем на каком уровне расположена кнопка
//                RemoteNodePtr remoteNode = remoteNodes_.at(row);// Нода, которая расположена на том же уровне
//                QMessageBox *mbTest = new QMessageBox;
//                mbTest->setText(QString("%1 %2").arg(QString::number(row,16)).arg(pushButton->objectName()));// Проверка урвня и имени ноды
//                mbTest->show();

//                alpha::protort::protocol::Packet_Payload backup;// Создаем пакет
//                backup.mutable_deploy_packet()->set_kind(alpha::protort::protocol::deploy::BackupTransition);// Устанавливаем тип пакета

//                remoteNode->async_backup_transition(backup);// Вызываем функцию, которая отправит этот пакет ноде.
//                return true;
//            }

    return QMainWindow::eventFilter(object, event);
}

void MainWindow::on_backup_transition()
{
    QPushButton *pushButton = qobject_cast<QPushButton*>(sender());
    if(pushButton->isEnabled())
    {
        int row = sender()->property("row").toInt();// У отправителя сигнала узнаем на каком уровне расположена кнопка
        RemoteNodePtr remoteNode = remoteNodes_.at(row);// Нода, которая расположена на том же уровне
        QMessageBox *mbTest = new QMessageBox;
        mbTest->setText(QString("%1 %2").arg(QString::number(row,16)).arg(sender()->objectName()));// Проверка урoвня и имени ноды
        mbTest->show();

        alpha::protort::protocol::Packet_Payload backup;// Создаем пакет
        backup.mutable_deploy_packet()->set_kind(alpha::protort::protocol::deploy::BackupTransition);// Устанавливаем тип пакета

        remoteNode->async_backup_transition(backup);// Вызываем функцию, которая отправит этот пакет ноде.
    }
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

void MainWindow::on_status_triggered()
{
    if (deploying)
        return;

    ui->treeStatus->expandAll();
    alpha::protort::protocol::Packet_Payload status;// ERRROR!
    status.mutable_deploy_packet()->set_kind(alpha::protort::protocol::deploy::GetStatus);

    for (auto &remoteNode: remoteNodes_)
    {
        remoteNode->async_status(status);
    }
}

QString MainWindow::fixedWindowTitle(const Document *doc) const
{
    QString result = doc->fileName();

    switch(doc->kind())
    {
    case Document::Kind::App:
        return result;
    case Document::Kind::Deploy:
        return result;
    default:
        result = tr("Безымянный");
        break;
    }

    for (int i = 0; ; ++i)
    {
        if (i > 1)
            result.chop(1);
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

    deploying = true;
    remoteNodes_.clear();

    for (auto node : deploy_config_.map_node)
    {
        auto remoteNode = boost::make_shared<RemoteNode>(node.second);
        remoteNodes_.append(remoteNode);

        connectRemoteNodeSignals(remoteNode.get());

        remoteNode->init(service_);
    }

    static_cast<TreeModel*>(ui->treeStatus->model())->setupModelData(remoteNodes_);

    m_statusTimer->start(500);
}

void MainWindow::connectRemoteNodeSignals(RemoteNode *node)
{
    connect(node, &RemoteNode::connected, this, &MainWindow::onConnected);
    connect(node, &RemoteNode::connectionFailed, this, &MainWindow::onConnectionFailed);
    connect(node, &RemoteNode::deployConfigRequestFinished, this, &MainWindow::onDeployConfigRequestFinished);
    connect(node, &RemoteNode::statusRequestFinished, this, &MainWindow::onStatusRequestFinished);
    connect(node, &RemoteNode::startRequestFinished, this, &MainWindow::onStartRequestFinished);
    connect(node, &RemoteNode::stopRequestFinished, this, &MainWindow::onStopRequestFinished);
    connect(node, &RemoteNode::backupTransitionRequestFinished, this, &MainWindow::onBackupTransitionRequestFinished);
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
    ui->tabWidget->setTabToolTip(ui->tabWidget->currentIndex(), doc->filePath());
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
    QLabel *app = new QLabel(tr("Описание: "));
    QLabel *schema = new QLabel(tr("Схема: "));

    m_apps = new QComboBox;
    m_deploys = new QComboBox;

    m_setupConfig = new QToolButton;
    m_setupConfig->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_setupConfig->setText(tr("Загрузить"));
    m_setupConfig->setIcon(QIcon(":/images/configure.ico"));

    QWidget *w = new QWidget;
    QHBoxLayout *l = new QHBoxLayout;
    l->addWidget(app);
    l->addWidget(m_apps);
    l->addWidget(schema);
    l->addWidget(m_deploys);
    l->addWidget(m_setupConfig);
    l->setContentsMargins(0, 0, 0, 0);
    w->setLayout(l);

    addWidgetOnBar(w);

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
void MainWindow::activateStatus() const
{
        ui->status->setEnabled(true);
}

void MainWindow::button_clickedSetup()
{
    if (m_apps->count()> 0 && m_deploys->count() > 0) {
        setupConfigMembers();
        setActiveConfig();
        createRemoteNodes();
    }
}

void MainWindow::setActiveConfig()
{
    for (int i = 0; i != ui->tabWidget->count(); ++i)
        setIcon(document(i));
    setTabIco(m_app,":/images/greenPen.png");
    setTabIco(m_deploySchema,":/images/greenCog.png");
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
    //    ui->statusLog->append(message);
}

Document *MainWindow::currentDocument(QComboBox *combobox)
{
    return qobject_cast<Document *>(qvariant_cast<QTextEdit *>(combobox->currentData()));
}

void MainWindow::setCurrentDocument(QComboBox * combobox, Document *doc)
{
    combobox->setCurrentIndex(combobox->findData(QVariant::fromValue(doc)));
}
