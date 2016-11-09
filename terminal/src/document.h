#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QDialog>
#include <QFile>

namespace UI {
class Document;
}

class Document
{
    QFile file;
    int type;
    void set_type();
public:
    Document();
    Document (QString fname);
    Document (Document &doc);
    Document operator=(Document &some);
    ~Document();
    QString name();
    QFile get_file();
    int get_type();
};

#endif // DOCUMENT_H
