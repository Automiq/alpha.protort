#include "mainwindow.h"
#include <QApplication>
#include <QSyntaxHighlighter>

int main(int argc, char *argv[])
{
    //Q_INIT_RESOURCE(editabletreemodel);

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
