#ifndef NODE_H
#define NODE_H

#include <iostream>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/chrono.hpp>

#include "packet.pb.h"
#include "server.h"
#include "client.h"
#include "node_settings.h"
#include "node_router.h"

namespace alpha {
namespace protort {
namespace node {

using namespace alpha::protort::link;

/*!
 * \brief Класс сетевого узла
 */
class node
{
public:
    node(const node_settings &settings)
        : client_(*this, service_),
          server_(*this, service_),
          settings_(settings),
          signals_(service_, SIGINT, SIGTERM)
    {
    }

    void start()
    {
        switch (settings_.component_kind)
        {
            case alpha::protort::protocol::Terminator:
            {
                signals_.async_wait(boost::bind(&io_service::stop,&service_));
                server_.listen(settings_.source);
                break;
            }
            case alpha::protort::protocol::Generator:
            {
                client_.async_connect(settings_.destination);
                break;
            }
        }
        service_.run();
    }

    /*!
     * \brief Уведомление об отправке пакета в канал связи
     * \param err
     * \param bytes
     */
    void on_packet_sent(const boost::system::error_code & err, size_t bytes)
    {
        // TODO
    }

    /*!
     * \brief Уведомление о подключении или безуспешном подключении
     * \param err
     */
    void on_connected(const boost::system::error_code & err)
    {
        // TODO
    }

    /*!
     * \brief Уведомление о приеме сервером новогопакета
     * \param buffer
     * \param nbytes
     */
    void on_new_packet(char const *buffer, size_t nbytes)
    {
        // TODO
    }

    /*!
     * \brief Уведомление о приеме сервером нового подключения
     * \param err
     */
    void on_new_connection(const boost::system::error_code & err)
    {
        // TODO
    }

private:
    //! I/O сервис
    io_service service_;

    //! Сервер
    link::server<node> server_;

    //! Список клиентов
    std::vector<link::client<node> > client_;

    //! Настройки узла
    node_settings settings_;

    //! Подписанные сигналы
    signal_set signals_;

    alpha::protort::node::node_router router_;

    friend class node_deploy;
};

} // namespace node
} // namespace protort
} // namespace alpha

#endif // NODE_H
