#ifndef PROTOCONNECTION_H
#define PROTOCONNECTION_H

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <iostream>

#include "../../link/src/packet_header.h"
#include "packet.pb.h"
#include "protocol.pb.h"

namespace alpha {
namespace protort {
namespace protolink {

using protocol_payload = alpha::protort::protocol::Packet::Payload;
using alpha::protort::link::packet_header;
using alpha::protort::link::max_packet_size;
using alpha::protort::link::header_size;

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
    static ptr new_(Callback& callback, boost::asio::io_service& service)
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
    boost::asio::ip::tcp::socket& sock()
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

    void send_response(protocol_payload& payload, int id)
    {
        protocol::Packet packet;
        packet.set_kind(protocol::Packet::Kind::Packet_Kind_Response);
        packet.mutable_transaction()->set_id(id);
        packet.mutable_payload()->CopyFrom(payload);
        async_send(packet.SerializeAsString());
    }

private:

    /*!
     * \brief Конструктор
     * \param callback Ссылка на объект, реализующий концепцию Callback
     * \param service Ссылка на io_service
     */
    connection(Callback& callback, boost::asio::io_service& service)
        : socket_(service),
          callback_(callback),
          packet_buffer_(new char[max_packet_size])
    {
#ifdef _DEBUG
        std::cout << "protoconnection constructor" << "\n";
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
     * \brief Колбек, вызываемый по окончании чтения заголовка
     * \param err Ошибка (если есть)
     * \param bytes Прочитанный размер полученного заголовка пакета в байтах
     */
    void on_header_read(const error_code& err, size_t bytes)
    {
        // TODO: handle disconnect
        if (err)
            return;

        // Закрываем соединение при размере пакета превышающем максимально допустимое
        if (packet_header_.packet_size > max_packet_size)
        {
#ifdef _DEBUG
            std::cout << "protoconnection has been terminated" << "\n";
#endif
            socket_.close();
            return;
        }

        do_read_packet();
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
                    boost::asio::buffer(packet_buffer_.get(), packet_size),
                    boost::asio::transfer_exactly(packet_size),
                    boost::bind(&connection::on_packet_read,
                                this->shared_from_this(),
                                boost::asio::placeholders::error,
                                boost::asio::placeholders::bytes_transferred));
    }

    /*!
     * \brief Колбек, вызываемый по окончании чтения пакета
     * \param err Ошибка (если есть)
     * \param bytes Прочитанный размер полученного пакета в байтах
     */
    void on_packet_read(const error_code& err, size_t bytes)
    {
        // TODO: handle disconnect
        if (err)
            return;

        // Уведомляем о новом пакете
        on_new_packet(packet_buffer_.get(), bytes);

        // Начинаем прием следующего пакета с чтения заголовка
        do_read_header();
    }

    void on_new_packet(char const *buffer, size_t nbytes)
    {
        protocol::Packet packet;
        packet.ParseFromArray(buffer, nbytes);

        switch (packet.kind()) {

        case protocol::Packet::Kind::Packet_Kind_Response:
            assert(false);
            break;

        case protocol::Packet::Kind::Packet_Kind_Request:            
        {
            protocol_payload response_payload = callback_.on_new_request(packet.payload());
#ifdef _DEBUG
            std::cout << "response packet id " << packet.transaction().id() << std::endl;
#endif
            send_response(response_payload, packet.transaction().id());
            break;
        }

        case protocol::Packet::Kind::Packet_Kind_Message:
            callback_.on_new_message(packet.payload());
            break;
        }
    }

    void async_send(const std::string& packet)
    {
        // Формируем заголовок
        auto header = reinterpret_cast<packet_header *>(packet_buffer_.get());
        header->packet_size = packet.size();

        // Копируем отправляемую строку в буффер
        copy(packet.begin(), packet.end(), packet_buffer_.get() + header_size);

        // Добавляем асинхронный таск на отправку
        async_write(
                    socket_,
                    boost::asio::buffer(packet_buffer_.get(), header_size + packet.size()),
                    boost::bind(&connection::on_packet_sent, this,
                                boost::asio::placeholders::error,
                                boost::asio::placeholders::bytes_transferred));
    }

    void on_packet_sent(const error_code& err, size_t bytes)
    {
        if (boost::asio::error::eof == err || boost::asio::error::connection_reset == err)
        {
            stop();
           // do_connect(ep_);
        }
        else
            callback_.on_packet_sent(err, bytes);
    }

    //! Сокет
    boost::asio::ip::tcp::socket socket_;

    //! Заголовок текущего пакета
    link::packet_header packet_header_;

    //! Буфер для текущего пакета
    std::unique_ptr<char> packet_buffer_;

    //! Ссылка на объект, предоставляющий callback-функции
    Callback& callback_;
};

} // namespace protolink
} // namespace protort
} // namespace alpha

#endif // PROTOCONNECTION_H
