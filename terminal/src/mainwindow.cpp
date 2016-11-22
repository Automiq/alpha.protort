#include <QFileDialog>
#include <QTextEdit>
#include <QObject>
#include <QTextStream>
#include <set>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "factory.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    service_(),
    work_(new boost::asio::io_service::work(service_)),
    protoThread_(boost::bind(&boost::asio::io_service::run, &service_))
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    work_.reset();
    service_.stop();
    if (protoThread_.joinable())
        protoThread_.join();
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
    on_deploy_triggered();
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

    alpha::protort::protocol::Packet_Payload payload_;
    payload_.mutable_deploy_packet()->set_kind(alpha::protort::protocol::deploy::Start);

    for (auto &client : clients_for_configuration)
    {
        client.second->async_send_request(payload_)->on_finished.connect
            (
                [&](alpha::protort::protocol::Packet_Payload p)
                  {
                     QMetaObject::invokeMethod(
                     this,
                     "on_finished",
                     Qt::QueuedConnection,
                     Q_ARG(alpha::protort::protocol::Packet_Payload, p));
                  }
            );
    }
}

void MainWindow::on_stop_triggered()
{
    ui->start->setEnabled(true);
    ui->status_request->setDisabled(true);
    ui->stop->setDisabled(true);

    alpha::protort::protocol::Packet_Payload payload_;
    payload_.mutable_deploy_packet()->set_kind(alpha::protort::protocol::deploy::Stop);

    for (auto &client : clients_for_configuration)
    {
        client.second->async_send_request(payload_)->on_finished.connect
            (
                [&](alpha::protort::protocol::Packet_Payload p)
                  {
                     QMetaObject::invokeMethod(
                     this,
                     "on_finished",
                     Qt::QueuedConnection,
                     Q_ARG(alpha::protort::protocol::Packet_Payload, p));
                  }
            );
    }
}

void MainWindow::on_deploy_triggered()
{
    ui->deploy->setDisabled(true);
    ui->start->setEnabled(true);

    {
        alpha::protort::parser::configuration config_;
        config_.parse_app(m_app.toStdString());
        config_.parse_deploy(m_deploySchema.toStdString());

        deploy_config_.parse_deploy(config_);
    }

    for (auto node : deploy_config_.map_node)
    {
        boost::asio::ip::tcp::endpoint ep(
                    boost::asio::ip::address::from_string(node.second.address), node.second.port); // Поменять порт на дефолтный
        std::unique_ptr<alpha::protort::protolink::client<MainWindow>> client_(
            new alpha::protort::protolink::client<MainWindow>(*this,service_));

        client_->node_name_ = node.second.name;

        clients_for_configuration.emplace(node.second.name,std::move(client_));
        clients_for_configuration.begin()->second->async_connect(ep);
    }
}

void MainWindow::on_connected(const boost::system::error_code& err, const std::string& current_node_)
{
    if (!err)
    {
        std::set<std::string> added_nodes_;
        std::set<std::string> added_maps_;

        alpha::protort::protocol::Packet_Payload payload_;

        payload_.mutable_deploy_packet()->set_kind(alpha::protort::protocol::deploy::DeployConfig);
        alpha::protort::protocol::deploy::Config* configuration_ =
            payload_.mutable_deploy_packet()->mutable_request()->mutable_deploy_config()->mutable_config();

        alpha::protort::protocol::deploy::NodeInfo* node_info_ = configuration_->add_node_infos();
        node_info_->set_name(current_node_);
        node_info_->set_port(deploy_config_.map_node[current_node_].port);
        node_info_->set_address(deploy_config_.map_node[current_node_].address);

        for (auto &component : deploy_config_.map_node_with_components[current_node_])
        {
            alpha::protort::protocol::ComponentKind kind_ =
                alpha::protort::components::factory::get_component_kind(deploy_config_.map_components[component.comp_name].kind);

            // Добавляем компонент к конфигурации нода
            alpha::protort::protocol::deploy::Instance* instance_ = configuration_->add_instances();

            instance_->set_name(component.comp_name);
            instance_->set_kind(kind_);

            // Указываем, что компонент относится к текущему ноду
            alpha::protort::protocol::deploy::Map* components_map_ = configuration_->add_maps();

            components_map_->set_node_name(current_node_);
            components_map_->set_instance_name(component.comp_name);

            for (auto &connection : deploy_config_.map_component_with_connections[component.comp_name])
            {
                // Добавляем коннекшион к конфигурации нода
                alpha::protort::protocol::deploy::Connection* connection_ = configuration_->add_connections();

                connection_->mutable_source()->set_name(connection.source);
                connection_->mutable_source()->set_port(connection.source_out);

                connection_->mutable_destination()->set_name(connection.dest);
                connection_->mutable_destination()->set_port(connection.dest_in);

                // Получаем имя нода компонента назначения
                std::string node_name_ = deploy_config_.map_component_node[connection.dest].node_name;

                if(node_name_ != current_node_)
                {
                    alpha::protort::parser::node& node_ = deploy_config_.map_node[node_name_];

                    if(added_nodes_.find(node_.name) == added_nodes_.end())
                    {
                        // Добавляем информацию о ноде в конфигурацию
                        alpha::protort::protocol::deploy::NodeInfo* remote_node_info_ = configuration_->add_node_infos();

                        remote_node_info_->set_name(node_.name);
                        remote_node_info_->set_port(node_.port);
                        remote_node_info_->set_address(node_.address);
                        added_nodes_.insert(node_.name);
                    }

                    if(added_maps_.find(connection.dest) == added_maps_.end())
                    {
                        // Добавляем информацию о мэпинге удаленного компонента
                        alpha::protort::protocol::deploy::Map* components_map_ = configuration_->add_maps();

                        components_map_->set_node_name(node_.name);
                        components_map_->set_instance_name(connection.dest);
                        added_maps_.insert(connection.dest);
                    }
                }
            }
        }
        clients_for_configuration[current_node_]->async_send_request(payload_)->on_finished.connect
            (
                [&](alpha::protort::protocol::Packet_Payload p)
                  {
                     QMetaObject::invokeMethod(
                     this,
                     "on_finished",
                     Qt::QueuedConnection,
                     Q_ARG(alpha::protort::protocol::Packet_Payload, p));
                  }
            );
    }
}

void MainWindow::on_packet_sent(const boost::system::error_code& err, size_t bytes)
{

}

void MainWindow::on_new_packet(alpha::protort::protocol::Packet_Payload packet_)
{

}

void MainWindow::on_finished(alpha::protort::protocol::Packet_Payload packet_)
{
    ui->text_browser_status->insertPlainText("on_finished\n");
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
