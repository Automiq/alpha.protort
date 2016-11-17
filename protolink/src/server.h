#ifndef PROTOSERVER_H
#define PROTOSERVER_H

#include <iostream>

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "connection.h"

namespace alpha {
namespace protort {
namespace protolink {

/*!
 * \brief Класс сервера
 * \tparam Callback
 * Концепция описывающая набор колбеков для приема событий от сервера
 */
template<class Callback>
class server
{
public:
    /*!
     * \brief Конструктор
     * \param callback Ссылка на объект, реализующий концепцию Callback
     * \param service Ссылка на I/O сервис
     */
    server(Callback& callback, boost::asio::io_service& service):
        acceptor_(service),
        callback_(callback)
    {
    }

    /*!
     * \brief Конструктор
     * \param callback Ссылка на объект, реализующий концепцию Callback
     * \param service Ссылка на I/O сервис
     * \param ep Адрес для прослушивания входящих подключений
     */
    server(Callback& callback, boost::asio::io_service& service, boost::asio::ip::tcp::endpoint ep)
        : acceptor_(service),
          callback_(callback)
    {
        listen(ep);
    }

    /*!
     * \brief Начать пролушивание
     * \param ep Адрес для прослушивания входящих подключений
     */
    void listen(boost::asio::ip::tcp::endpoint ep)
    {
        // Настраиваем акцептор
        acceptor_.open(ep.protocol());
        acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
        acceptor_.bind(ep);
        acceptor_.listen();

        // Принимаем новое подключение
        do_accept_connection();
    }

private:

    //! Тип указателя для входящего соединения
    using connection_ptr = boost::shared_ptr<connection<Callback>>;

    /*!
     * \brief Асинхронно принять новое входящее подключение
     */
    void do_accept_connection()
    {
        // Создаем объект будущего подключения
        connection_ptr client = connection<Callback>::new_(
            callback_, acceptor_.get_io_service());

        // Принимаем подключение
        acceptor_.async_accept(
            client->sock(),
            boost::bind(&server::on_connection_accepted, this, client,
                        boost::asio::placeholders::error));
    }

    /*!
     * \brief Колбек, вызываемый при новом принятом входящем соединении
     * \param client Принятое входящее соединение
     * \param err Ошибка приема (если есть)
     */
    void on_connection_accepted(connection_ptr client, const boost::system::error_code& err)
    {
        // TODO: handle error
        if (err)
            return;

        // Запускаем соединение
        client->start();

        // Уведомляем о новом подключении
        callback_.on_new_connection(err);

        // Принимаем следующее подключение
        do_accept_connection();
    }

    //! Акцептор входящих подключений
    boost::asio::ip::tcp::acceptor acceptor_;

    //! Ссылка на объект, предоставляющий callback-функции
    Callback& callback_;
};

} // namespace protolink
} // namespace protort
} // namespace alpha

#endif // PROTOSERVER_H
