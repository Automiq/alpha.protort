#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QComboBox>
#include <QList>
#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>
#include <QSyntaxHighlighter>

#include "configdialog.h"
#include "document.h"
#include "deploy.pb.h"
#include "configdialog.h"

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

    void setTabName(int index, const QString &name);

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

private:
    Ui::MainWindow *ui;
    ConfigDialog *dlg;
    QComboBox *m_apps;
    QComboBox *m_deploys;
    QPushButton *m_setupConfig;
    QString m_app;
    QString m_deploySchema;
    std::vector<StatusResponse> m_statOut;
    QAbstractItemModel *status;

    QString fixedWindowTitle(const Document *doc) const;
    void saveDocument(int index);
    void activateDeploy() const;
    void addDocument(Document *doc);
    void addWidgetOnBar(QWidget* newWidget) const;
    void setIcon(Document *doc);
    void setupWindowConfigurations();
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
    void deployOk();
    void save_session();
    void load_session();
    void load_file(const QString& fileName);
    Document* document(int index);
};

#endif // MAINWINDOW_H
