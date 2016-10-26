#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QSyntaxHighlighter>
#include <QMainWindow>

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

    void on_load_description_triggered();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
