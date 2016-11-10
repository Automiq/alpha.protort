#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QDialog>
#include <QFile>
#include <QTextEdit>
#include <QByteArray>

namespace Ui {
class Document;
}

class Document: public QTextEdit
{
    Q_OBJECT

public:
    Document(QWidget *parent = 0);
    void load();
    void save();
    QString fileName() const;
    void setFileName(const QString &fileName);
    int kind() const;

private:
    QString name;
};

#endif // DOCUMENT_H
