#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSyntaxHighlighter>
#include <QMessageBox>
#include <QList>
#include <map>
#include <boost/thread.hpp>

#include "document.h"
#include "deploy.pb.h"
#include "configdialog.h"
#include "client.h"
#include "parser.h"

class QTextEdit;

namespace Ui {
class MainWindow;
}

using namespace alpha::protort::protocol::deploy;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);

    ~MainWindow();

    void setTabName(int index, const QString& name);

    void on_packet_sent(const boost::system::error_code& err, size_t bytes);

    void on_connected(const boost::system::error_code& err, const std::string& node_name_);

    void on_new_packet(alpha::protort::protocol::Packet_Payload packet_);

    void on_finished(alpha::protort::protocol::Packet_Payload packet_);

private slots:

    void on_save_file_triggered();

    void on_save_all_triggered();

    void on_exit_triggered();

    void on_load_file_triggered();

    void on_create_file_triggered();

    void on_tabWidget_tabCloseRequested(int index);

    void on_config_triggered();

    void on_start_triggered();

    void on_stop_triggered();

    void on_deploy_triggered();

    void on_close_file_triggered();

    void close_tab(int index);

    void on_status_request_triggered();

private:
    Ui::MainWindow *ui;
    QString m_app;
    QString m_deploySchema;
    void saveDocument(int index);
    void addDocument(Document *doc);
    QString fixedWindowTitle(const Document *doc) const;
    std::vector<StatusResponse> stat_out;

    boost::asio::io_service service_;

    std::map<std::string,std::unique_ptr<alpha::protort::protolink::client<MainWindow>>> clients_for_configuration;

    alpha::protort::parser::configuration config;

    boost::thread protoThread_;
};

#endif // MAINWINDOW_H
