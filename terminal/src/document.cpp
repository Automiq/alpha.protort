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
    setFont(QFont("Consolas", 10));
}

bool Document::save()
{
    if(m_name.isEmpty())
        m_name = getFileNameOFD();

    if(!m_name.isEmpty())
    {
        QFile file(m_name);

        if (file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            file.write(toPlainText().toUtf8());
            file.close();
            return true;
       }
    }
    return false;
}

QString Document::filePath() const
{
    return m_name;
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

void Document::setFileName(const QString &fileName)
{
    m_name = fileName;
}

bool Document::isApp() const
{
    return Kind::App == kind();
}

bool Document::isDeploy() const
{
    return Deploy == kind();
}

bool Document::isUnknown() const
{
    return Unknown == kind();
}

QString Document::getFileNameOFD()
{
    return QFileDialog::getSaveFileName(this, tr("Сохранить файл как..."),
                                        QString(),
                                        tr("Описание приложения/Схема развёртывания (*.xml);; Все типы (*.*)"));
}
