#include "document.h"

enum doc_type
{
    undef,
    deploy,
    app
};

void Document::set_type()
{
    QFile tmp(name());
    QString type_str = "";
    if (this->get_file().open(QIODevice::ReadOnly))
    {
        type_str = tmp.readLine();
        if (type_str == "<deploy>")
            type = deploy;
        else if (type_str == "<app>")
            type = app;
        else
            type = undef;
    }
}

Document::Document()
{
    type = undef;
}

Document::Document(QString fname)
{
    file.setFileName(fname);
    set_type();
}

Document::Document(Document &doc)
{
    file.setFileName(doc.file.fileName());
    type = doc.type;
}

Document Document::operator=(Document &some)
{
    Document res(some);
    return res;
}

Document::~Document()
{
    file.close();
    file.~QFile();
}

int Document::get_type(){ return type; }

QString Document::name(){ return file.fileName(); }
