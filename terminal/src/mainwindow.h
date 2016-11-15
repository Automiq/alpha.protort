#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSyntaxHighlighter>
#include "configdialog.h"
#include <QMessageBox>
#include <QList>
#include "document.h"

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

    void saveDocument();

    void addDocument(Document *doc);

    Document* currentDocument() const;

    QString fixedWindowTitle(const Document *doc) const;

private:
    Ui::MainWindow *ui;
    QString m_app;
    QString m_deploySchema;
};

#endif // MAINWINDOW_H
