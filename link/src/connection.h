#ifndef CONNECTION_H
#define CONNECTION_H

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <iostream>

#include "packet_header.h"

namespace alpha {
namespace protort {
namespace link {

using namespace boost::asio;

/*!
 * \brief Класс входящего соединения
 *
 * \tparam Callback
 */
template<class Callback> class connection :
        public boost::enable_shared_from_this<connection<Callback>>,
        boost::noncopyable
{
public:
    using error_code = boost::system::error_code;
    using ptr = boost::shared_ptr<connection<Callback>>;

    /*!
     * \brief Создать новое соединение
     *
     * \param callback Ссылка на объект, реализующий концепцию Callback
     * \param service Ссылка на I/O сервис
     * \return Новый объект класса соединение
     */
    static ptr new_(Callback& callback, io_service& service)
    {
        return ptr(new connection(callback, service));
    }

    /*!
     * \brief Запуск соединения
     */
    void start()
    {
        // Начинаем прием пакетов с чтения заголовка первого пакета
        do_read_header();
    }

    /*!
     * \brief Останов соединения
     */
    void stop()
    {
        // Закрываем сокет
        socket_.close();
    }

    /*!
     * \brief Получить ссылку на сокет
     */
    ip::tcp::socket& sock()
    {
        return socket_;
    }

    /*!
     * \brief Деструктор
     */
    ~connection()
    {
        stop();
    }

private:

    /*!
     * \brief Конструктор
     * \param callback Ссылка на объект, реализующий концепцию Callback
     * \param service Ссылка на io_service
     */
    connection(Callback& callback, io_service& service)
        : socket_(service),
          callback_(callback),
          read_buffer_(new char[max_packet_size])
    {
#ifdef _DEBUG
        std::cout << "server constructor" << "\n";
#endif
    }

    /*!
     * \brief Асинхронный метод чтения хедера очередного пакета
     */
    void do_read_header()
    {
        async_read(
            socket_,
            boost::asio::buffer(&packet_header_, header_size),
            boost::asio::transfer_exactly(header_size),
            boost::bind(&connection::on_header_read,
                        this->shared_from_this(),
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
    }

    /*!
     * \brief Асинхронный метод чтения пакета
     */
    void do_read_packet()
    {
        // Размер пакета согласно заголовку
        auto packet_size = packet_header_.packet_size;

        // Добавим асинхронный таск чтения пакета заданного размера
        async_read(
            socket_,
            boost::asio::buffer(read_buffer_.get(), packet_size),
            boost::asio::transfer_exactly(packet_size),
            boost::bind(&connection::on_packet_read,
                        this->shared_from_this(),
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
    }

    /*!
     * \brief Колбек, вызываемый по окончании чтения заголовка
     * \param err Ошибка (если есть)
     * \param bytes Прочитанный размер полученного заголовка пакета в байтах
     */
    void on_header_read(const error_code& ec, size_t bytes)
    {
        // TODO: handle disconnect
        if (ec)
            return;

        do_read_packet();
    }

    /*!
     * \brief Колбек, вызываемый по окончании чтения пакета
     * \param err Ошибка (если есть)
     * \param bytes Прочитанный размер полученного пакета в байтах
     */
    void on_packet_read(const error_code& ec, size_t bytes)
    {
        // TODO: handle disconnect
        if (ec)
            return;

        // Уведомляем о новом пакете
        callback_.on_new_packet(read_buffer_.get(), bytes);

        // Начинаем прием следующего пакета с чтения заголовка
        do_read_header();
    }

    //! Сокет
    ip::tcp::socket socket_;

    //! Заголовок текущего пакета
    packet_header packet_header_;

    //! Буфер для текущего пакета
    std::unique_ptr<char> read_buffer_;

    //! Ссылка на объект, предоставляющий callback-функции
    Callback& callback_;
};

} // namespace link
} // namespace protort
} // namespace alpha

#endif // CONNECTION_H
