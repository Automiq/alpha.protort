#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSyntaxHighlighter>

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
    void on_open_file_triggered();

    void on_save_file_triggered();

    void on_save_as_triggered();

    void on_exit_triggered();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
