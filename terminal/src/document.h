#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QDialog>
#include <QFile>

namespace UI {
class Document;
}

class Document
{
    QString name;
    int type;

public:
    Document();
    Document (QString name);
    ~Document();
    int get_type();
    void parse_type();
};

#endif // DOCUMENT_H
