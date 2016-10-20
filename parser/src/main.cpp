#include "parser.h"

int main(int argc, char *argv[])
{
    try
    {
        Deploy_scheme ds;
        ds.parse_app("app.xml");
        ds.parse_deploy("deploy.xml");

    }
    catch(std::exception &e)
    {
        std::cout << "Error: " << e.what() << std::endl;
    }

    return 0;
}

