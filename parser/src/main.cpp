#include "parser.h"

int main(int argc, char *argv[])
{
    Deploy_scheme ds;
    const std::string app_path = "./app.xml";
    ds.parse_app(app_path);
    const std::string deploy_path = "./deploy.xml";
    ds.parse_deploy(deploy_path);
    component comp =  ds.getComponent(std::string("A"));
    node n = ds.getNode(comp);
    std::cout << "component " << comp.name << " of type " << comp.type
              <<" is mapped to " << n.id << " " << n.ip << ":" << n.port << std::endl;
    return 0;
}
