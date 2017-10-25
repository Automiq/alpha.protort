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

//Деструктор главного окна.
MainWindow::~MainWindow()
{
	//Цикл проверяет открытые файлы (вкладки) и в случае если они не сохранены предлагает пользователю сохранить их.
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
    //После цикла идет сохранение сессии, отсановка потока service (основной поток ожидает его завершения методом join)
    save_session();
    work_.reset();
    service_.stop();
    if (serviceThread_.joinable())
        serviceThread_.join();
    delete ui;
}

//Сохранение настроек сессии. (Сериализация открытых файлов)
//Создается экземпляр QTшного QSettings (файл настроек), в которые записываются пути открытых на данный момент в терминале файлов.
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

//Загрузка сессии, процесс обратнтый процессу сохранения (MainWindow::save_session())
//Читаем пути файлов из файла настроек и открываем их в терминале.
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

//Обработка кнопки сохранения (голубая дискета Ctrl+S).
//Сохраняет текущий (видимую вкладку) открытый файл.
void MainWindow::on_save_file_triggered()
{
    saveDocument(ui->tabWidget->currentIndex());
}

//Обработка кнопки сохранения файлов (две черные дискеты Ctrl+Shift+S)
//Сохряняет все открытые в терминале файлы.
void MainWindow::on_save_all_triggered()
{
    for (int i = 0; i < ui->tabWidget->count(); ++i)
        saveDocument(i);
}

//Обработка кнопки "выход" Ctrl+Q
void MainWindow::on_exit_triggered()
{
    ui->stop->triggered(true);
    close();
}

//Открытие файла. Обязательный параметр - путь к файлу.
//Внутри метода выполняются проверки на пустоту, существование файла.
//В случае прохода всех проверок, на основе файла создается экземпляр Document
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

//Обработка кнопки "Открыть"" Ctrl+O.
//По нажатию открывается диалоговое окно выбора файла с нужных форматов.
//После выбора файла и нажатия ОК диалоговое окно закрывается и файл открывается.
void MainWindow::on_load_file_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Открыть файл"), QString(),
                                                    tr("Описание приложения/Схема развёртывания (*.xml);; Все типы (*)"));
    load_file(fileName);
}

//Удаление документа из комбобокса (например "Описание" - m_apps или "Cхема" - m_deploys)
//Первый параметр - из какого комбобока удалить, второй параметр - какой док удалить из комбобокса.
void MainWindow::delDocFromComboBox(QComboBox* combobox, Document* doc)
{
    combobox->removeItem(combobox->findData(QVariant::fromValue(doc)));
}

//Удаление конфига из обоих комбобоксов. Параметр - какой док удаляем.
void MainWindow::delConfig(Document *doc)
{
    delDocFromComboBox(m_apps, doc);
    delDocFromComboBox(m_deploys, doc);
}

//Удаление дока и добавления его заного. См описания delConfig и addConfig.
void MainWindow::updateConfig(Document *doc)
{
    delConfig(doc);
    addConfig(doc);
}

//Обработка кнопки создания нового файла (Ctrl+N)
void MainWindow::on_create_file_triggered()
{
    Document *tmp = new Document();
    addDocument(tmp);
}

//Обработка закртия вкладки (файла).
void MainWindow::on_tabWidget_tabCloseRequested(int index)
{
    close_tab(index);
}

//Добавить конфиг. Внутри проверяется нет ли уже такого конфига (файл развертки и описания, отсюда два if`а).
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

//Установка активного документа в комбобоксе. Параметр 1 - какой комбобокс, параметр 2 - какой документ.
void MainWindow::setComboBoxToolTip(QComboBox *combobox, Document *doc)
{
     combobox->setItemData(combobox->findData(QVariant::fromValue(doc)), doc->filePath(), Qt::ToolTipRole);
}

//Вызывает диалоговое окно предлогающее выбрать deploy и app схемы из ОТКРЫТЫХ файлов.
//Выбранные устанавливает как активные (и делает их иконки зелеными)
//Пошарившись в терминале я не смог найти как вызвать этот диалог.
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

//Делаем кнопку start доступной для нажатия
void MainWindow::resetDeployActions() const
{
    if (!ui->start->isEnabled() && !ui->stop->isEnabled())
        ui->start->setEnabled(true);
}

void MainWindow::showLog() const
{
}

//Логирование успешности развертки конфигурации на узел
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

//Обработка респонса о статусе ноды. Запись в нижную таблицу новых данных или запись ошибки в лог ошибки.
//ATTENTION на данный момент метод writeStatusLog реализации не имеет!
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

        writeStatusLog(tr("<Название компонента - %1>")
                       .arg(QString::fromStdString(component.name())));
        writeStatusLog(tr("<Количество принятых пакетов - %1>")
                       .arg(QString::number(component.in_packet_count())));
        writeStatusLog(tr("<Количество переданных пакетов - %1>")
                       .arg(QString::number(status.component_statuses(i).out_packet_count())));
    }

    writeStatusLog("\r\n");
}

//Логирование успешности запуска узла
void MainWindow::onStartRequestFinished(const alpha::protort::protocol::deploy::Packet& packet)
{
    auto node = qobject_cast<RemoteNode *>(sender());
    writeLog(
                packet.has_error() ?
                    tr("Ошибка запуска узла %1").arg(node->info()) :
                    tr("Узел %1 успешно запущен").arg(node->info())
                    );
}

//Логирование успешности остановки узла
void MainWindow::onStopRequestFinished(const alpha::protort::protocol::deploy::Packet& packet)
{
    auto node = qobject_cast<RemoteNode *>(sender());
    writeLog(
                packet.has_error() ?
                    tr("Ошибка останова узла %1").arg(node->info()) :
                    tr("Узел %1 успешно остановлен").arg(node->info())
                    );
}

//Лог при успешном подключении к ноде. И активация кнопок (enabled).
void MainWindow::onConnected()
{
    auto node = qobject_cast<RemoteNode *>(sender());
    writeLog(tr("Успешное подключение к узлу %1").arg(node->info()));
    activateDeploy();
    activateStatus();
}

//Лог при не удачном подключении к ноде.
void MainWindow::onConnectionFailed(const boost::system::error_code& err)
{
    auto node = qobject_cast<RemoteNode *>(sender());
    writeLog(tr("Невозможно подключиться к %1: %2")
             .arg(node->info())
             .arg(QString::fromStdString(err.message())));
}

//Устанавливает имя вкладки. Параметр1 - индекс вкладки, параметр2 - путь к файлу.
void MainWindow::setTabName(int index, const QString &name)
{
    ui->tabWidget->setTabText(index, QString(QFileInfo(name).fileName()));
}

//При нажатии кнопки запуск делаем её не доступной, а кнопку стоп доступной.
//Потом всем нодам отправляем Payload пакет с командой старта.
void MainWindow::on_start_triggered()
{
    ui->start->setDisabled(true);
    ui->stop->setEnabled(true);

    alpha::protort::protocol::Packet_Payload payload;
    payload.mutable_deploy_packet()->set_kind(alpha::protort::protocol::deploy::Start);

    for (auto &remoteNode: remoteNodes_)
        remoteNode->async_start(payload);
}

//При нажатии кнопки стоп делаем её не доступной, а кнопку старт доступной.
//Потом всем нодам отправляем Payload пакет с командой остановки.
void MainWindow::on_stop_triggered()
{
    ui->start->setEnabled(true);
    ui->stop->setDisabled(true);

    alpha::protort::protocol::Packet_Payload payload;
    payload.mutable_deploy_packet()->set_kind(alpha::protort::protocol::deploy::Stop);

    for (auto &remoteNode: remoteNodes_)
        remoteNode->async_stop(payload);
}

//Вызов меседжбокса с вопросом о загрузке новой конфигруации при запущенной старой.
//По ответу да - деплой().
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

//Развертка. Посылаем нодам конфигурацию развертки (см. parse_deploy)
void MainWindow::deploy()
{

    resetDeployActions();
    ui->status->setEnabled(true);
    ui->deploy->setDisabled(true);

    for (auto &remoteNode: remoteNodes_)
        remoteNode->async_deploy(deploy_config_);

    TreeModel *model = new TreeModel(remoteNodes_);
    model->setupModelData(remoteNodes_);

    ui->treeStatus->show();
}

//При попытки развертки в случае когда конфигурация сменилась вспылвает меседжбокс "точно ли мы этого хотим".
void MainWindow::on_deploy_triggered()
{
    if(ui->start->isEnabled() || ui->stop->isEnabled())
        showMessage();
    else
        deploy();
}

//Закрытие текущей вкладки. (При закрытии файла)
void MainWindow::on_close_file_triggered()
{
    close_tab(ui->tabWidget->currentIndex());
}

//Закрытие вкладки по индексу документа и удаление из комбобоксов
void MainWindow::close_tab(int index)
{
    auto doc = document(index);
    delConfig(doc);
    ui->tabWidget->widget(index)->deleteLater();
    ui->tabWidget->removeTab(index);
}

//Получения статусов нод. Обработка кнопки Статус.
//Посылаем на ноды Payload пакет о запросе статуса.
void MainWindow::on_status_triggered()
{
    if (deploying)
        return;

    ui->treeStatus->expandAll();
    alpha::protort::protocol::Packet_Payload status;
    status.mutable_deploy_packet()->set_kind(alpha::protort::protocol::deploy::GetStatus);

    for (auto &remoteNode: remoteNodes_)
        remoteNode->async_status(status);
}

//Установка названия вкладки файла. Параметр - документ.
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

//Даем команду парсеру парсить (lol) файлы выбранные комбобоксами app и deploy файлы.
//По итогу парсинга получаем структуру с заполненными данынми.
//Инициализируем каждую ноду и добавляем её в список remoteNodes_
//На основе списка создаем дерево (TreeModel)
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

//Связь нодов с GUI методами, в основном для записи лога.
void MainWindow::connectRemoteNodeSignals(RemoteNode *node)
{
    connect(node, &RemoteNode::connected, this, &MainWindow::onConnected);
    connect(node, &RemoteNode::connectionFailed, this, &MainWindow::onConnectionFailed);
    connect(node, &RemoteNode::deployConfigRequestFinished, this, &MainWindow::onDeployConfigRequestFinished);
    connect(node, &RemoteNode::statusRequestFinished, this, &MainWindow::onStatusRequestFinished);
    connect(node, &RemoteNode::startRequestFinished, this, &MainWindow::onStartRequestFinished);
    connect(node, &RemoteNode::stopRequestFinished, this, &MainWindow::onStopRequestFinished);
}

//Сохранение документа
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

//Добавление документа (создание вкладки документа)
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

//Установка иконки вкладке. Параметр1 -  какому документу, параметр2 - путь к картинке.
void MainWindow::setTabIco(Document *doc, const QString &srcPath) const
{
    ui->tabWidget->setTabIcon(ui->tabWidget->indexOf(doc), QIcon(srcPath));
}

//Устанавливает документу иконку, в зависимости от его типа.
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

//Добавление элемента на главнную форму
void MainWindow::addWidgetOnBar(QWidget* newWidget) const
{
    ui->mainToolBar->addWidget(newWidget);
}

//создание на главной форме тулбара конфигурации (два комбобокса для app и deploy + кнопака загрузить)
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

//Задаем лейблу текущеий документ комбобокса
void MainWindow::setupConfigMembers()
{
    m_app = currentDocument(m_apps);
    m_deploySchema = currentDocument(m_deploys);
}

//Делаем кнопку deploy доступной для нажатия
void MainWindow::activateDeploy() const
{
    if(!ui->deploy->isEnabled() && m_deploys->count() && m_apps->count())
        ui->deploy->setEnabled(true);
}

//Делаем кнопку status доступной для нажатия
void MainWindow::activateStatus() const
{
        ui->status->setEnabled(true);
}

//Обработка клика кнопки "Загрузить"
void MainWindow::button_clickedSetup()
{
    if (m_apps->count()> 0 && m_deploys->count() > 0) {
        setupConfigMembers();
        setActiveConfig();
        createRemoteNodes();
    }
}

//Подвечивает иконки файлов заленым (активным).
//Например при нажаитии на кнопку "загрузить"
void MainWindow::setActiveConfig()
{
    for (int i = 0; i != ui->tabWidget->count(); ++i)
        setIcon(document(i));
    setTabIco(m_app,":/images/greenPen.png");
    setTabIco(m_deploySchema,":/images/greenCog.png");
}

//Возвращает указатель обьекта документ по индексу открытой вкладки
Document* MainWindow::document(int index)
{
    return dynamic_cast<Document*> (ui->tabWidget->widget(index));
}

//Запись соотбщения в деплойлог. Параметр - сообщение
void MainWindow::writeLog(const QString &message)
{
    ui->deployLog->append(message);
}

void MainWindow::writeStatusLog(const QString &message)
{
    //    ui->statusLog->append(message);
}

//Возвращает текущий документ (текущий в комбобоксе). Параметр - комбобокс
Document *MainWindow::currentDocument(QComboBox *combobox)
{
    return qobject_cast<Document *>(qvariant_cast<QTextEdit *>(combobox->currentData()));
}

//Устанавливает документ текущим (параметр2) в комбобоксе (параметр1), 
void MainWindow::setCurrentDocument(QComboBox * combobox, Document *doc)
{
    combobox->setCurrentIndex(combobox->findData(QVariant::fromValue(doc)));
}