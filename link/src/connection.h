#ifndef LINK_CONNECTION_H
#define LINK_CONNECTION_H

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "common_header.h"

namespace alpha {
namespace protort {
namespace link {

using namespace boost::asio;

/*!
 * Шаблонный класс соединения
 */
template<class Callback> class connection :
        public boost::enable_shared_from_this<connection<Callback>>,
        boost::noncopyable
{
public:
    using error_code = boost::system::error_code;
    using ptr = boost::shared_ptr<connection<Callback>>;

    /*!
     * \brief start Метод для старта чтения пакета
     */
    void start()
    {
#ifdef _DEBUG
        std::cout << "start" << "\n";
#endif
        read_header();
    }

    /*!
     * \brief new_  Статическая функция возвращает объект класса connection
     * \param callback Ссылка на объект, реализующий концепцию Callback
     * \param service Ссылка на io_service
     * \return Новый объект класса connection
     */
    static ptr new_(Callback& callback,io_service& service)
    {
        ptr new_(new connection(callback,service));
        return new_;
    }

    /*!
     * \brief stop Метод для закрытия соединения
     */
    void stop()
    {
#ifdef _DEBUG
        std::cout << "server connection has been stopped\n";
#endif
        sock_.close();
    }

    /*!
     * \brief sock Метод возвращает объект сокета
     * \return Объекта класса socket
     */
    ip::tcp::socket & sock()
    {
        return sock_;
    }

    ~connection()
    {
        stop();
    }

private:

    /*!
     * \brief connection Конструктор, инициализация объектов сокета и колбека
     * \param callback Ссылка на объект, реализующий концепцию Callback
     * \param service Ссылка на io_service
     */
    connection(Callback& callback,io_service& service):
        sock_(service),
        callback_(callback),
        read_buffer_(new char[max_packet_size])
    {
#ifdef _DEBUG
        std::cout << "server constructor" << "\n";
#endif
    }

    /*!
     * \brief ready_for_new_packet Метод для вызовал callback функции
     *  и начала чтения нового пакета
     * \param err Отлавливание ошибки
     * \param bytes Размер пришедшего пакета в байтах
     */
    void ready_for_new_packet(const error_code & err, size_t bytes)
    {
        callback_.on_new_packet(read_buffer_.get(), bytes);
        read_header();
    }

    /*!
     * \brief read_packet Метод для чтения тела пакета
     * \param err Отлавливание ошибки
     * \param bytes Размер заголовка в байтах
     */
    void read_packet(const error_code & err, size_t bytes)
    {
#ifdef _DEBUG
        std::cout << "server on_read " << err << "\n";
#endif
        if (!err)
        {
            auto header = reinterpret_cast<packet_header *>(read_buffer_.get());
#ifdef _DEBUG
            std::cout << " size: " << header->packet_size << "\n";
#endif
            auto packet_size = header->packet_size;
            async_read(
                sock_,
                boost::asio::buffer(read_buffer_.get(), packet_size),
                boost::asio::transfer_exactly(packet_size),
                boost::bind(&connection::ready_for_new_packet, this->shared_from_this(),_1,_2));
        }
    }

    /*!
     * \brief read_header Метод для чтения хедера пакета
     */
    void read_header()
    {
#ifdef _DEBUG
        std::cout << "server do_read" << "\n";
#endif
        async_read(
            sock_,
            boost::asio::buffer(read_buffer_.get(), header_size),
            boost::asio::transfer_exactly(header_size),
            boost::bind(&connection::read_packet, this->shared_from_this(),_1,_2));
    }

    ip::tcp::socket sock_;
    std::unique_ptr<char> read_buffer_;
    Callback& callback_;
};

} //namespace link
} //namespace protort
} //namespace alpha

#endif // LINK_CONNECTION_H
