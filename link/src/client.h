#ifndef LINK_CLIENT_H
#define LINK_CLIENT_H

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include "common_header.h"

using namespace boost::asio;

namespace alpha {
namespace protort {
namespace link {

/*
 *Шаблонный класс клиента
 *
 */
template<class Callback> class client
{
    using error_code = boost::system::error_code;

public:

    //Конструктор класса, в котором происходит инициализация сокета и колбека
    client(Callback &callback, boost::asio::io_service& service)
        :sock_(service),
         callback_(callback)
    {

    }

    //Конструктор класса, в котором происходит инициализация сокета и колбека,
    //а также вызывается метод для асинхронного подключения
    client(Callback &callback, boost::asio::io_service& service, ip::tcp::endpoint ep)
        :sock_(service),
         callback_(callback)
    {
        async_connect(ep);
    }

    //Метод для асинхронного подключения
    void async_connect(ip::tcp::endpoint ep)
    {
        sock_.async_connect(ep, boost::bind(&client::on_connect, this, boost::asio::placeholders::error));
    }

    //Метод для отправки пакета данных
    void async_send(std::string const& msg)
    {
        do_write(msg);
    }

    //Деструктор, в котором закрывается соединение
    ~client()
    {
        std::cout << "dctor client" << std::endl;
        stop();
    }

private:

    ip::tcp::socket sock_;
    char header_buffer_[header_size];
    char write_buffer_[buffer_size];
    Callback& callback_;

    //Метод, для проверки подключения и вызова колбека
    void on_connect(const error_code & err)
    {
        std::cout << "on_connect " << err << std::endl;
        if (!err)
            callback_.on_connected();
        else
            stop();
    }

    //Метод для закрытия соединения
    void stop()
    {
#ifdef _DEBUG
        std::cout << "client connection has been stopped\n";
#endif
        sock_.close();
    }

    //Метод для отлавливания ошибок отправки пакета
    void on_write(const error_code & err, size_t bytes)
    {
#ifdef _DEBUG
        std::cout << "on_write " << err << " and " << bytes << "\n";
#endif
    }

    //Метод для отправки сообщения
    void do_write(const std::string& msg)
    {
#ifdef _DEBUG
        std::cout << "Sended from client: " << msg << "\n";
#endif
        auto header = reinterpret_cast<packet_header *>(write_buffer_);
        header->packet_size = msg.size();
        copy(msg.begin(), msg.end(), write_buffer_ + header_size);
        sock_.async_write_some(
            buffer(write_buffer_, msg.size() + header_size),
            boost::bind(&client::on_write,this,_1,_2));
    }
};

} // namespace link
} // namespace protort
} // namespace alpha

#endif // LINK_CLIENT_H
