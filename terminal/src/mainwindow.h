#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSyntaxHighlighter>
#include <QMessageBox>
#include <QList>
#include <vector>
#include <boost/thread.hpp>
#include <QThread>
#include <boost/asio.hpp>

#include "document.h"
#include "deploy.pb.h"
#include "configdialog.h"
#include "remotenode.h"
#include "deploy_configuration.h"

class QTextEdit;

namespace Ui {

class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);

    ~MainWindow();

    void setTabName(int index, const QString& name);

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

    boost::asio::io_service service_;

    boost::scoped_ptr<boost::asio::io_service::work> work_;

    std::vector<boost::shared_ptr<RemoteNode>> terminal_clients;

    alpha::protort::parser::deploy_configuration deploy_config_;

    boost::thread protoThread_;
};

#endif // MAINWINDOW_H
