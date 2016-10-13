#include "node_settings.h"


int main(int argc, const char *argv[])
{
    alpha::protort::node::node_settings node_settings;
    node_settings.parse(argc, argv);


    return 0;
}
