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
    QFont f("Courier", 10);
    f.setStyleHint(QFont::Courier);
    f.setStyleStrategy(QFont::PreferAntialias);
    setFont(f);
}

//Сохранение документа - запись на диск. m_name - путь к файлу.
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

//Геттер пути к файлу документа.
QString Document::filePath() const
{
    return m_name;
}

//Геттер имени файла документа.
QString Document::fileName() const
{
    return QFileInfo(m_name).fileName();
}

//Возвращает тип (kind) документа (app, deploy, Unknown)
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

//Сеттер пути к файлу. Параметр - путь.
void Document::setFilePath(const QString &fileName)
{
    m_name = fileName;
}

//bool проверка. Документ типа app?
bool Document::isApp() const
{
    return Kind::App == kind();
}

//bool проверка. Документ типа deploy?
bool Document::isDeploy() const
{
    return Deploy == kind();
}

//bool проверка. Документ типа unknown?
bool Document::isUnknown() const
{
    return Unknown == kind();
}

//Диалог сохранения документа как файла
QString Document::getFileNameOFD()
{
    return QFileDialog::getSaveFileName(this, tr("Сохранить файл как..."),
                                        QString(),
                                        tr("Описание приложения/Схема развёртывания (*.xml);; Все типы (*.*)"));
}
