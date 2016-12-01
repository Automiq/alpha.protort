#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QComboBox>
#include <QList>
#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>
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
#include "configdialog.h"
#include "remotenode.h"
#include "deployconfiguration.h"

class QTextEdit;

namespace Ui {

class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    using RemoteNodePtr = boost::shared_ptr<RemoteNode>;

public:
    explicit MainWindow(QWidget *parent = 0);

    ~MainWindow();

    void setTabName(int index, const QString &name);
    static Document *current_doc(QComboBox *combobox);
    static void set_current_index(QComboBox * combobox, Document *doc);

public slots:
    void button_clickedSetup();

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

public slots:
    void showLog() const;

private slots:
    void onDeployConfigRequestFinished(const alpha::protort::protocol::deploy::Packet& packet);
    void onStatusRequestFinished(const alpha::protort::protocol::deploy::Packet& status);
    void onStartRequestFinished(const alpha::protort::protocol::deploy::Packet& packet);
    void onStopRequestFinished(const alpha::protort::protocol::deploy::Packet& packet);
    void onConnected();
    void onConnectionFailed(const boost::system::error_code&);

private:
    Ui::MainWindow *ui;
    ConfigDialog *dlg;
    QComboBox *m_apps;
    QComboBox *m_deploys;
    QPushButton *m_setupConfig;
    Document *m_app = nullptr;
    Document *m_deploySchema = nullptr;

    QString fixedWindowTitle(const Document *doc) const;
    void saveDocument(int index);
    void activateDeploy() const;
    void addDocument(Document *doc);
    void addWidgetOnBar(QWidget* newWidget) const;
    void setIcon(Document *doc);
    void createConfigurationToolBar();
    void addConfig(Document *doc);
    void delConfig(Document *doc);
    void deleteConfig(QComboBox *ptr, const QString &name);
    void updateConfig(Document *doc);
    void setActiveConfig();
    void setupActiveIcon(const int &index);
    void setupConfigMembers();
    void setTabIco(Document *doc, const QString &srcPath) const;
    void resetDeployActions() const;
    void showMessage();
    void deploy();
    void createRemoteNodes();
    void connectRemoteNodeSignals(RemoteNode* node);

    Document* document(int index);

    void writeLog(const QString& message);
    void writeStatusLog(const QString& message);

    boost::asio::io_service service_;

    boost::scoped_ptr<boost::asio::io_service::work> work_;

    QList<RemoteNodePtr> remoteNodes_;

    deploy_configuration deploy_config_;

    boost::thread serviceThread_;
};

#endif // MAINWINDOW_H
