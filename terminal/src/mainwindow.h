#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSyntaxHighlighter>
#include "configdialog.h"
#include <QMessageBox>
#include "document.h"
#include "deploy.pb.h"

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

    void on_Status_request_triggered();

private:
    Ui::MainWindow *ui;
    QTextEdit *createNewTab(const QString &name);
    QString *openFiles;
    QString m_app;
    QString m_deploySchema;
};

#endif // MAINWINDOW_H
