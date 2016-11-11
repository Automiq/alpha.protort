#include "document.h"
#include <QWidget>
#include <QFileDialog>
#include <QXmlStreamReader>
#include <QMessageBox>
#include <QTextStream>
#include <QByteArray>

Document::Document(QWidget *parent)
    : QTextEdit(parent)
{
}

void Document::save()
{
    QFile sfile(this->name);
    if ( sfile.open(QIODevice::ReadWrite) )
    {
        QTextStream stream( &sfile );
        stream << this->toPlainText() << endl;
        sfile.close();
    }
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

    if (xml.name() == "app")
        return Kind::App;

    if (xml.name() == "deploy")
        return Kind::Deploy;

    return Kind::Unknown;
}
