#include "document.h"

enum doc_type
{
    undef,
    deploy,
    app
};

void Document::parse_type()
{
    QFile file(name);
    QString type_str = "";
    if (file.open(QIODevice::ReadOnly))
    {
        type_str = file.readLine();
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
    name = "";
    type = undef;
}

Document::Document(QString name)
{
    this->name = name;
    parse_type();
}

int Document::get_type()
{
    return type;
}
