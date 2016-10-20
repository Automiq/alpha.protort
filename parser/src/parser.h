#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include <string>
#include <map>
#include <set>
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
struct connection
{
    component source;
    component dest;
    short source_out;
    short dest_in;
};
bool operator <(connection const& left, connection const& right)
{
    return left.source.name < right.source.name;
}

struct node
{
    std::string id;
    std::string ip;
    short port;
};

class Deploy_scheme
{
public:
    void parse_app(const std::string &filename);
    void parse_deploy(const std::string &filename);
    component& getComponent(std::string& id)
    {
        return id_to_component[id];
    }
    node& getNode(component& comp)
    {
        return component_to_node[comp];
    }

private:
    std::map<std::string, component> id_to_component;
    std::map<std::string, node> id_to_node;
    std::set<connection> connections;
    std::map<component, node> component_to_node;
};

void Deploy_scheme::parse_app(const std::string &filename)
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
            connection conn;
            conn.source = id_to_component[v.second.get<std::string>("<xmlattr>.source")];
            conn.source_out = v.second.get<short>("<xmlattr>.source_out");
            conn.dest = id_to_component[v.second.get<std::string>("<xmlattr>.dest")];
            conn.dest_in = v.second.get<short>("<xmlattr>.dest_in");
            connections.insert(conn);
        }
        else
            std::cout << "Unknown tag in the file" << std::endl;
    }
}
void Deploy_scheme::parse_deploy(const std::string &filename)
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
            id_to_node.insert(std::pair<std::string, node>(n.id, n));
        }
        else if( v.first == "map" ) {

            component comp = id_to_component[v.second.get<std::string>("<xmlattr>.instance")];
            node n = id_to_node[v.second.get<std::string>("<xmlattr>.node")];
            component_to_node.insert(std::pair<component, node>(comp, n));
        }
        else
            std::cout << "Unknown tag in the file" << std::endl;
    }
}


#endif // PARSER_H
