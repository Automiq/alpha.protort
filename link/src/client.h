#ifndef LINK_CLIENT_H
#define LINK_CLIENT_H

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include "common_header.h"

using namespace boost::asio;

namespace alpha {
namespace protort {
namespace link {

/*!
 * Шаблонный класс клиента
 */
template<class Callback> class client
{
    using error_code = boost::system::error_code;

public:

    /*!
     * \brief client Конструктор класса, в котором происходит инициализация сокета и колбека
     * \param callback Ссылка на объект, реализующий концепцию Callback
     * \param service Ссылка на io_service
     */
    client(Callback &callback, boost::asio::io_service& service)
        :sock_(service),
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
    client(Callback &callback, boost::asio::io_service& service, ip::tcp::endpoint ep)
        :sock_(service),
         callback_(callback),
         write_buffer_(new char[max_packet_size + header_size])
    {
        async_connect(ep);
    }

    /*!
     * \brief async_connect Метод для асинхронного подключения
     * \param ep Объект класса endpoint
     */
    void async_connect(ip::tcp::endpoint ep)
    {
        sock_.async_connect(ep, boost::bind(&client::on_connect, this, boost::asio::placeholders::error));
    }

    /*!
     * \brief async_send Метод для отправки пакета данных
     * \param msg Строка для отправки
     */
    void async_send(std::string const& msg)
    {
        do_write(msg);
    }

    ~client()
    {
        stop();
    }

private:

    ip::tcp::socket sock_;    
    std::unique_ptr<char> write_buffer_;
    Callback& callback_;

    /*!
     * \brief on_connect Метод, для проверки подключения и вызова колбека
     * \param err Отлавливание ошибки
     */
    void on_connect(const error_code & err)
    {
#ifdef _DEBUG
        std::cout << "on_connect " << err << std::endl;
#endif
        if (!err)
            callback_.on_connected();
        else
            stop();
    }

    /*!
     * \brief stop Метод для закрытия соединения
     */
    void stop()
    {
#ifdef _DEBUG
        std::cout << "client connection has been stopped\n";
#endif
        sock_.close();
    }

    /*!
     * \brief on_write Метод для отлавливания ошибок отправки пакета
     * \param err Отлавливание ошибки
     * \param bytes Размер посланного пакета в байтах
     */
    void on_write(const error_code & err, size_t bytes)
    {
#ifdef _DEBUG
        std::cout << "on_write " << err << " and " << bytes << "\n";
#endif
    }

    /*!
     * \brief do_write Метод для отправки сообщения
     * \param msg Строка для отправки
     */
    void do_write(const std::string& msg)
    {
#ifdef _DEBUG
        std::cout << "Sended from client: " << msg << "\n";
#endif
        auto header = reinterpret_cast<packet_header *>(write_buffer_.get());
        header->packet_size = msg.size();
        copy(msg.begin(), msg.end(), write_buffer_.get() + header_size);
        sock_.async_write_some(
            buffer(write_buffer_.get(), msg.size() + header_size),
            boost::bind(&client::on_write,this,_1,_2));

    }
};

} // namespace link
} // namespace protort
} // namespace alpha

#endif // LINK_CLIENT_H
