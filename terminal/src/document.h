#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QDialog>
#include <QFile>
#include <QTextEdit>

namespace Ui {
class Document;
}

class Document: public QTextEdit
{
    Q_OBJECT

public:

    enum Kind
    {
        Unknown,
        Deploy,
        App
    };

    Document(QWidget *parent = 0);
    void load();
    bool save();

    QString fileName() const;
    Kind kind() const;
    void setFileName(const QString &fileName);

private:

    QString getFileNameOFD();
    QString m_name;
};

#endif // DOCUMENT_H
