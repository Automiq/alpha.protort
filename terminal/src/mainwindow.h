#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QComboBox>
#include <QList>
#include <QMainWindow>
#include <QMessageBox>
#include <QSyntaxHighlighter>

#include "configdialog.h"
#include "document.h"
#include "deploy.pb.h"



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
    QComboBox *m_apps;
    QComboBox *m_deploys;
    QString m_app;
    QString m_deploySchema;
    std::vector<StatusResponse> m_statOut;

    QString fixedWindowTitle(const Document *doc) const;
    void saveDocument(int index);
    void addDocument(Document *doc);
    void setIcon(Document *doc);
    void setupWindowConfigurations();
    void addConfig(QString &name, QComboBox *ptr);
    void defineToAddConf(Document *doc);
    void delConfig(Document *doc);
    void deleteConfig(QComboBox *ptr, QString &nameD);
    void updateConfig(Document *doc, Document::Kind before, Document::Kind after);
};

#endif // MAINWINDOW_H
