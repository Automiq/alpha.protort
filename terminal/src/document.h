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

    void setFileName(const QString &fileName);
    QString getFileNameOFD();

    QString fileName() const;
    Kind kind() const;


private:
    QString name;
};

#endif // DOCUMENT_H
