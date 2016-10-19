#ifndef LINK_SERVER_H
#define LINK_SERVER_H

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "common_header.h"

namespace alpha {
namespace protort {
namespace link {

using namespace boost::asio;

/*
 * Шаблонный класс соединения
*/
template<class Callback> class connection :
        public boost::enable_shared_from_this<connection<Callback>>,
        boost::noncopyable
{
public:
    using error_code = boost::system::error_code;
    using ptr = boost::shared_ptr<connection<Callback>>;

    //Метод для старта чтения пакета
    void start()
    {
#ifdef _DEBUG
        std::cout << "start" << "\n";
#endif
        read_header();
    }

    //Статическая функция возвращает объект класса connection
    static ptr new_(Callback& callback,io_service& service)
    {
        ptr new_(new connection(callback,service));
        return new_;
    }

    //Метод для закрытия соединения
    void stop()
    {
#ifdef _DEBUG
        std::cout << "server connection has been stopped\n";
#endif
        sock_.close();
    }

    //Метод возвращает объект сокета
    ip::tcp::socket & sock()
    {
        return sock_;
    }

    //Деструктор, в котором закрывается соединение
    ~connection()
    {
        stop();
    }

private:

    //Конструктор, инициализация объектов сокета и колбека
    connection(Callback& callback,io_service& service):
        sock_(service),
        callback_(callback)
    {
#ifdef _DEBUG
        std::cout << "server constructor" << "\n";
#endif
    }

    void ready_for_new_packet(const error_code & err, size_t bytes)
    {
        std::cout << "ewrtry" << err << std::endl;
        callback_.on_new_packet(read_buffer_, bytes);
        read_header();
    }

    //Метод для чтения тела пакета
    void read_packet(const error_code & err, size_t bytes)
    {
#ifdef _DEBUG
        std::cout << "server on_read " << err << "\n";
#endif
        if (!err)
        {
            auto header = reinterpret_cast<packet_header *>(read_buffer_);
#ifdef _DEBUG
            std::cout << " size: " << header->packet_size << "\n";
#endif
            auto packet_size = header->packet_size;
            async_read(
                sock_,
                boost::asio::buffer(read_buffer_),
                boost::asio::transfer_exactly(packet_size),
                boost::bind(&connection::ready_for_new_packet, shared_from_this(),_1,_2));
        }
    }

    //Метод для чтения хедера пакета
    void read_header()
    {
#ifdef _DEBUG
        std::cout << "server do_read" << "\n";
#endif
        async_read(
            sock_,
            boost::asio::buffer(read_buffer_),
            boost::asio::transfer_exactly(header_size),
            boost::bind(&connection::read_packet, shared_from_this(),_1,_2));
    }

    ip::tcp::socket sock_;
    char read_buffer_[buffer_size];
    Callback& callback_;
};


/*
 * Шаблонный класс сервера
 */

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

    //Конструктор, в котором происходит инициализация аксептора и колбека,
    //а также вызывается метод listen для старта прослушивания
    server (Callback& callback,io_service& service,ip::tcp::endpoint ep):
        acceptor_(serveice),
        callback_(callback)
    {
        listen(ep);
    }

    //Метод для старта прослушивания
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

    //Метод, в котором стартует чтение пакета и начинается ожидание нового подключения
    void handle_accept(connection_ptr client, const boost::system::error_code & err)
    {
        std::cout << "handle accept" << std::endl;
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
