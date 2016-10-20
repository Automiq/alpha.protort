#ifndef LINK_SERVER_H
#define LINK_SERVER_H

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include "common_header.h"
#include "connection.h"

namespace alpha {
namespace protort {
namespace link {

using namespace boost::asio;

/*!
 * \brief Класс сервера
 * \tparam Callback
 * Концепция описывающая набор колбеков для приема событий от сервера
 */
template<class Callback> class server
{
    using connection_ptr = boost::shared_ptr<connection<Callback>>;
public:

    /*!
     * \brief server Конструктор, в котором происходит инициализация аксептора и колбека
     * \param callback Ссылка на объект, реализующий концепцию Callback
     * \param service Ссылка на io_service
     */
    server (Callback& callback,io_service& service):
        acceptor_(service),
        callback_(callback)
    {
        std::cout << "ctr" << std::endl;
    }

    /*!
     * \brief server Конструктор, в котором происходит инициализация аксептора и колбека
     * а также вызывается метод listen для старта прослушивания
     * \param callback Ссылка на объект, реализующий концепцию Callback
     * \param service Ссылка на io_service
     * \param ep Объект endpoint
     */
    server (Callback& callback,io_service& service,ip::tcp::endpoint ep):
        acceptor_(serveice),
        callback_(callback)
    {
        listen(ep);
    }

    /*!
     * \brief listen Метод для старта прослушивания
     * \param ep Объект endpoint
     */
    void listen(ip::tcp::endpoint ep)
    {
        acceptor_.open(ep.protocol());
        acceptor_.set_option(ip::tcp::acceptor::reuse_address(true));
        acceptor_.bind(ep);
        acceptor_.listen();
        connection_ptr client = connection<Callback>::new_(callback_,acceptor_.get_io_service());
        acceptor_.async_accept(
            client->sock(),
            boost::bind(&server::handle_accept,this,client,boost::asio::placeholders::error));
    }

private:

    /*!
     * \brief handle_accept Метод, в котором стартует чтение пакета и начинается ожидание нового подключения
     * \param client Объект класса connection
     * \param err Отлавливание ошибки
     */
    void handle_accept(connection_ptr client, const boost::system::error_code & err)
    {
        client->start();
        callback_.on_new_connection();
        connection_ptr new_client = connection<Callback>::new_(callback_,acceptor_.get_io_service());
        acceptor_.async_accept(
            new_client->sock(),
            boost::bind(&server::handle_accept,this,new_client,boost::asio::placeholders::error));
    }

    ip::tcp::acceptor acceptor_;
    Callback& callback_;
};

} //namespace link
} //namespace protort
} //namespace alpha

#endif // LINK_SERVER_H