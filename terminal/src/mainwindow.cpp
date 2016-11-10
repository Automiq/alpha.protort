#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QTextEdit>
#include <QObject>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::close_tab(int index)
{
    ui->tabWidget->currentWidget()->deleteLater();
    ui->tabWidget->removeTab(index);
    if (ui->tabWidget->count() == 0)
        close();
}

void MainWindow::on_save_file_triggered()
{
    auto text_edit = dynamic_cast<QTextEdit*> (ui->tabWidget->currentWidget());
    QFile file(ui->tabWidget->tabText(ui->tabWidget->currentIndex()));
    if (file.open(QIODevice::ReadWrite))
    {
        file.write(text_edit->toPlainText().toUtf8());
        file.close();
    }
}

void MainWindow::on_save_all_triggered()
{
    for (int i = ui->tabWidget->count(); i >= 0; --i)
    {
        auto text_edit = dynamic_cast<QTextEdit*> (ui->tabWidget->widget(i));
        if (!text_edit)
            continue;
        QFile file(ui->tabWidget->tabText(i));
        if (file.open(QIODevice::WriteOnly))
        {
            file.write(text_edit->toPlainText().toUtf8());
            file.close();
        }
    }
}

void MainWindow::on_exit_triggered()
{
    ui->stop->triggered(true);
    close();
}

void MainWindow::on_load_file_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, QString ("Открыть файл"), QString(), QString("xml (*.xml);; all (*)"));

    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this,
                            tr("File error"),
                            tr("Failed to open\n%1").arg(fileName));
        return;
    }
    Document *tmp = new Document();
    tmp->setText(file.readAll());
    tmp->setFileName(fileName);
    addDocument(tmp);
}

void MainWindow::addDocument(Document *doc)
{
    if (ui->tabWidget->indexOf(doc) != -1)
        return;
    ui->tabWidget->addTab(doc, fixedWindowTitle(doc));
}

QString MainWindow::fixedWindowTitle(const Document *doc) const
{
    QString title = doc->fileName();

    if (title.isEmpty())
        title = tr("Unnamed");
    else
        title = QFileInfo(title).fileName();

    QString result;

    for (int i = 0; ; ++i)
    {
        result = title;
        if (i > 0)
            result += QString::number(i);

        bool unique = true;
        for (int j = 0; j < ui->tabWidget->count(); ++j)
        {
            const QWidget *widget = ui->tabWidget->widget(j);
            if (widget == doc)
                continue;
            if (result == ui->tabWidget->tabText(j))
            {
                unique = false;
                break;
            }
        }

        if (unique)
            break;
    }
    return result;
}


void MainWindow::on_create_file_triggered()
{
    QString file_name = QFileDialog::getSaveFileName(this, QString ("Создать файл"), QString(), QString("xml (*.xml);; all (*)"));
    QFile file(file_name);

    if (file.open(QIODevice::ReadWrite))
    {
        if (ui->tabWidget->tabText(ui->tabWidget->currentIndex()) == "Tab 1")
            ui->tabWidget->setTabText(ui->tabWidget->currentIndex(), QString(QFileInfo(file_name).fileName()));
        else
            ui->tabWidget->setCurrentIndex(ui->tabWidget->addTab(new QTextEdit(), QString(QFileInfo(file_name).fileName())));
        file.close();
    }
}

void MainWindow::on_tabWidget_tabCloseRequested(int index)
{
    close_tab(index);
}

void MainWindow::on_config_triggered()
{
    ConfigDialog dlg(this);
    if (dlg.exec())
    {
        m_app = dlg.app();
        m_deploySchema = dlg.deploySchema();
        ui->start->setDisabled(true);
        ui->stop->setDisabled(true);
        ui->deploy->setEnabled(true);
    }
}

void MainWindow::on_start_triggered()
{
    ui->start->setDisabled(true);
    ui->stop->setEnabled(true);
}

void MainWindow::on_stop_triggered()
{
    ui->start->setEnabled(true);
    ui->stop->setDisabled(true);
}

void MainWindow::on_deploy_triggered()
{
    ui->deploy->setDisabled(true);
    ui->start->setEnabled(true);
}

void MainWindow::on_close_file_triggered()
{
    close_tab(ui->tabWidget->currentIndex());
}
/*
QTextEdit MainWindow::createNewTab(QString path)
{
    QTextEdit text_edit = new QTextEdit();
    tabWidget->setCurrentIndex(ui->tabWidget->addTab(text_edit, name));
    ui->tabWidget->setTabText(ui->tabWidget->currentIndex(), name);
    return text_edit;
}*/
