#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include <string>
#include <map>
#include <set>
#include <exception>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>

struct component
{
    std::string name;
    std::string type;
};
bool operator <(component const& left, component const& right)
{
    return left.name < right.name;
}

struct node
{
    std::string id;
    std::string ip;
    unsigned short port;
};

class Deploy_scheme
{
public:
    typedef std::pair<std::string, short> comp_name_port;
    typedef std::pair<comp_name_port, comp_name_port> connection;
    void parse_app(const std::string &filename);
    void parse_deploy(const std::string &filename);
    component& get_component(std::string& id)
    {
        return id_to_component[id];
    }
    std::string& get_node(std::string comp_name)
    {
        return component_to_node[comp_name];
    }
    comp_name_port get_dest_and_port(comp_name_port source_name_port)
    {
        return connections[source_name_port];
    }
private:
    std::map<std::string, component> id_to_component;
    std::map<std::string, node> id_to_node;
    std::map<comp_name_port, comp_name_port> connections;
    std::map<std::string, std::string> component_to_node;
};

void Deploy_scheme::parse_app(const std::string &filename)
{
    try
    {
        // Create an empty property tree object
        using boost::property_tree::ptree;
        boost::property_tree::ptree pt;

        // Load the XML file into the property tree. If reading fails
        // (cannot open file, parse error), an exception is thrown.
        read_xml(filename, pt);


        BOOST_FOREACH( ptree::value_type const& v, pt.get_child("app") ) {
            if( v.first == "instance" ) {
                component comp;
                comp.name = v.second.get<std::string>("<xmlattr>.name");
                comp.type = v.second.get<std::string>("<xmlattr>.kind");
                id_to_component.insert(std::pair<std::string, component>(comp.name, comp));
            }
            else if( v.first == "connection" ) {

                std::string source_name = v.second.get<std::string>("<xmlattr>.source");
                short source_out = v.second.get<short>("<xmlattr>.source_out");
                std::string dest_name = v.second.get<std::string>("<xmlattr>.dest");
                short dest_in = v.second.get<short>("<xmlattr>.dest_in");
                connections.insert(connection(comp_name_port(source_name, source_out), comp_name_port(dest_name, dest_in)));
            }
            else
                std::cout << "Unknown tag in the file" << std::endl;
        }
    }
    catch(std::exception &e)
    {
        std::cout << "Error: " << e.what() << std::endl;
    }


}
void Deploy_scheme::parse_deploy(const std::string &filename)
{
    try
    {

        // Create an empty property tree object
        using boost::property_tree::ptree;
        boost::property_tree::ptree pt;

        // Load the XML file into the property tree. If reading fails
        // (cannot open file, parse error), an exception is thrown.
        read_xml(filename, pt);


        BOOST_FOREACH( ptree::value_type const& v, pt.get_child("deploy") ) {
            if( v.first == "node" ) {
                node n;
                n.id = v.second.get<std::string>("<xmlattr>.name");
                n.ip = v.second.get<std::string>("<xmlattr>.address");
                n.port = v.second.get<unsigned short>("<xmlattr>.port");
                id_to_node.insert(std::pair<std::string, node>(n.id, n));
            }
            else if( v.first == "map" ) {

                std::string comp_name = v.second.get<std::string>("<xmlattr>.instance");
                std::string node_name = v.second.get<std::string>("<xmlattr>.node");
                component_to_node.insert(std::make_pair(comp_name, node_name));
            }
            else
                std::cout << "Unknown tag in the file" << std::endl;
        }


    }
    catch(std::exception &e)
    {
        std::cout << "Error: " << e.what() << std::endl;
    }
}
bool operator <(std::pair<std::string, short>& left, std::pair<std::string, short>& right)
{
    return left.first < right.first;
}

#endif // PARSER_H
