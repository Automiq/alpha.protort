#ifndef NODE_H
#define NODE_H

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/chrono.hpp>
#include <iostream>

#include "server.h"
#include "client.h"

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
    node()
        : client_(*this, service_),
          server_(*this, service_),
          signals_(service_, SIGINT, SIGTERM)
    {
    }

    void start()
    {

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
    boost::asio::io_service service_;

    //! Сервер
    link::server<node> server_;

    //! Клиент
    link::client<node> client_;

    //! Подписанные сигналы
    boost::asio::signal_set signals_;
};

} // namespace node
} // namespace protort
} // namespace alpha

#endif // NODE_H
