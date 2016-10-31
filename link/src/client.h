#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "packet_header.h"

namespace alpha {
namespace protort {
namespace link {

/*!
 * Шаблонный класс клиента
 */
template<class Callback>
class client
{
    using error_code = boost::system::error_code;

public:

    /*!
     * \brief client Конструктор класса, в котором происходит инициализация сокета и колбека
     * \param callback Ссылка на объект, реализующий концепцию Callback
     * \param service Ссылка на io_service
     */
    client(Callback &callback, boost::asio::io_service& service)
        : socket_(service),
          callback_(callback),
          write_buffer_(new char[max_packet_size + header_size])
    {
    }

    /*!
     * \brief client Конструктор класса, в котором происходит инициализация сокета и колбека,
     * а также вызывается метод для асинхронного подключения
     * \param callback Ссылка на объект, реализующий концепцию Callback
     * \param service Ссылка на объект, реализующий концепцию Callback
     * \param ep Объект класса endpoint
     */
    client(Callback &callback, boost::asio::io_service& service, boost::asio::ip::tcp::endpoint ep)
        : socket_(service),
          callback_(callback),
          write_buffer_(new char[header_size + max_packet_size])
    {
        async_connect(ep);
    }

    /*!
     * \brief Деструктор
     */
    ~client()
    {
        stop();
    }

    /*!
     * \brief stop Метод для закрытия соединения
     */
    void stop()
    {
        socket_.close();
    }

    /*!
     * \brief Метод для асинхронного подключения
     * \param ep Объект класса endpoint
     */
    void async_connect(boost::asio::ip::tcp::endpoint ep)
    {
        do_connect(ep);
    }

    /*!
     * \brief Метод для отправки пакета данных
     * \param msg Строка для отправки
     */
    void async_send(std::string const& msg)
    {
        do_send_packet(msg);
    }

private:

    /*!
     * \brief Выполнить подключение
     */
    void do_connect(boost::asio::ip::tcp::endpoint ep)
    {
        socket_.async_connect(
            ep, boost::bind(&client::on_connect, this,
                            boost::asio::placeholders::error));
    }

    /*!
     * \brief Послать пакет
     * \param msg Содержимое пакета
     */
    void do_send_packet(const std::string& packet)
    {
        // Формируем заголовок
        auto header = reinterpret_cast<packet_header *>(write_buffer_.get());
        header->packet_size = packet.size();

        // Копируем отправляемую строку в буффер
        copy(packet.begin(), packet.end(), write_buffer_.get() + header_size);

        // Добавляем асинхронный таск на отправку
        async_write(
            socket_,
            boost::asio::buffer(write_buffer_.get(), header_size + packet.size()),
            boost::bind(&client::on_packet_sent, this,
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
    }

    /*!
     * \brief Колбек, вызываемый по окончании попытки соединения
     *
     * \param err Ошибка соединения (если есть)
     */
    void on_connect(const error_code& err)
    {
        if (err)
        {
            stop();
            return;
        }

        callback_.on_connected(err);
    }

    /*!
     * \brief Колбек, вызываемый по окончании отправка пакета
     *
     * \param err Ошибка отправки (если есть)
     * \param bytes Размер отправленного пакета в байтах
     */
    void on_packet_sent(const error_code& err, size_t bytes)
    {
        callback_.on_packet_sent(err, bytes);
    }

    //! Сокет
    boost::asio::ip::tcp::socket socket_;

    //! Буфер для отправки пакета
    std::unique_ptr<char> write_buffer_;

    //! Ссылка на объект, предоставляющий callback-функции
    Callback& callback_;
};

} // namespace link
} // namespace protort
} // namespace alpha

#endif // CLIENT_H
