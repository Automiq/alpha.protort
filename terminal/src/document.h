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
    QString fileName() const;
    void setFileName(const QString &fileName);
    Kind kind() const;
    void save();

private:
    QString name;
};

#endif // DOCUMENT_H
