#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>
#include <map>
#include <set>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>


class Deploy_scheme
{
public:
    void load(const std::string &filename);

private:
    std::set<connection> connections;
    std::map<component, node> component_to_node;
};

Deploy_scheme::load(const std::string &filename)
{
        // Create an empty property tree object
        using boost::property_tree::ptree;
        boost::property_tree::ptree pt;

        // Load the XML file into the property tree. If reading fails
        // (cannot open file, parse error), an exception is thrown.
        read_xml(filename, pt);

        // Get the filename and store it in the m_file variable.
        // Note that we construct the path to the value by separating
        // the individual keys with dots. If dots appear in the keys,
        // a path type with a different separator can be used.
        // If the debug.filename key is not found, an exception is thrown.
        m_file = pt.get<std::string>("debug.filename");

        // Get the debug level and store it in the m_level variable.
        // This is another version of the get method: if the value is
        // not found, the default value (specified by the second
        // parameter) is returned instead. The type of the value
        // extracted is determined by the type of the second parameter,
        // so we can simply write get(...) instead of get<int>(...).
        m_level = pt.get("debug.level", 0);

        // Iterate over the debug.modules section and store all found
        // modules in the m_modules set. The get_child() function
        // returns a reference to the child at the specified path; if
        // there is no such child, it throws. Property tree iterators
        // are models of BidirectionalIterator.
        BOOST_FOREACH(ptree::value_type &v,
                pt.get_child("debug.modules"))
            m_modules.insert(v.second.data());
}

struct component
{
    std::string name;
    alpha::protort::protocol::Packet::ComponentKind type;
};

struct connection
{
    component source;
    component dest;
    short source_out;
    short dest_in;
};

struct node
{
    std::string id;
    std::string ip;
    short port;
};

#endif // PARSER_H
