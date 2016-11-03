#ifndef NODE_DEPLOY_H
#define NODE_DEPLOY_H

#include <iostream>
#include <map>
#include <vector>
#include "node.h"
#include "parser.h"
#include "components.h"

namespace alpha {
namespace protort {
namespace node {

/*!
 * \brief Разворачивает узел
 * Принимает конфигурацию приложения и развертывания и инициализирует роутер,
 * создавая необходимые компоненты, локальные и удаленные связи.
 */
class node_deploy
{
public:  
    /*!
     * \brief Преобразует конфигурацию в промежуточную структуру
     * \param conf Конфигурация, создаваемая xml парсером
     */
    void load_config(alpha::protort::parser::configuration &);

    /*!
     * \brief Разворачивает узел
     * \param _node Узел, на котором происходит разворачивание
     * Создает необходимые компоненты, локальные и удаленные связи роутера.
     */
    void deploy(node &);

private:
    /// Маршрут
    struct destination
    {
        std::string comp_name;
        unsigned short in_port;
    };

    /// Информация о компоненте
    struct component_info
    {
        std::string kind;
        std::string node_name;
        std::map<unsigned short, std::vector<destination> > connections;
    };

    /// Информация о узле
    struct node_info
    {
        std::string address;
        unsigned short port;
    };


    std::map<std::string, component_info> components_;
    std::map<std::string, node_info> nodes_;
    std::string current_node_name;
};

} // namespace node
} // namespace protort
} // namespace alpha

#endif // NODE_DEPLOY_H
