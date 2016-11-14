#include "document.h"
#include <QWidget>
#include <QFileDialog>
#include <QXmlStreamReader>
#include <QMessageBox>
#include <QTextStream>
#include <QByteArray>
#include <QTextCodec>
#include <QDebug>

Document::Document(QWidget *parent)
    : QTextEdit(parent)
{
    name = "";
}

bool Document::save()
{
    if(name == "")
        name = getFileNameOFD();

    if(name != "")
    {
        QFile sfile(name);

        if ( sfile.open(QIODevice::WriteOnly | QIODevice::Text) )
        {
            sfile.write(toPlainText().toUtf8());
            sfile.close();
        }
        return 0;
    }
    return 1;
}

QString Document::fileName() const
{
    return name;
}

void Document::setFileName(const QString &fileName)
{
    name = fileName;
}

Document::Kind Document::kind() const
{
    QXmlStreamReader xml(toPlainText());
    xml.readNextStartElement();

    if (xml.name() == "app")
        return Kind::App;

    if (xml.name() == "deploy")
        return Kind::Deploy;

    return Kind::Unknown;
}

QString Document::getFileNameOFD()
{
    return QFileDialog::getSaveFileName(this, tr("Сохранить файл как..."),
                                        QString(),
                                        tr("Описание приложения/Схема развёртывания (*.xml);; Все типы (*.*)"));
}
