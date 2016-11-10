#include "document.h"
#include <QWidget>
#include <QFileDialog>
#include <QXmlStreamReader>

enum doc_type
{
    undef,
    deploy,
    app
};

Document::Document(QWidget *parent)
    : QTextEdit(parent)
{
}

//QByteArray Document::txt(){return text;}

void Document::load()
{

}

QString Document::fileName() const
{
    return name;
}

void Document::setFileName(const QString &fileName)
{
    name = fileName;
}
/*
int Document::kind() const
{

    QString *device = "<app>"; // вот эту строку надо читать из таба
    QXmlStreamReader xml.setDevice(device);
    if (xml.readNextStartElement())
    {
        if (xml.name() == "app")
        return 0;
        if (xml.name() == "deploy")
        return 1;
    return 2;
    }
    return !xml.error();
}
*/
