#ifndef PROTOCLIENT_H
#define PROTOCLIENT_H

#include <iostream>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/signals2.hpp>
#include <map>
#include <boost/asio.hpp>

#include "packet.pb.h"
#include "protocol.pb.h"
#include "packet_header.h"

namespace alpha {
namespace protort {
namespace link {

/*!
 * \brief Временной интервал для повторного подключения
 *
 * Временной интервал, через который будет выполнена повторная попытка подключения
 * если предыдущая попытка не увенчалась успехом.
 */
static const int proto_reconnect_interval = 5000;

/*!
 * Шаблонный класс клиента
 */
template<class Callback>
class protoclient
{
    using error_code = boost::system::error_code;

public:

    /*!
     * \brief client Конструктор класса, в котором происходит инициализация сокета и колбека
     * \param callback Ссылка на объект, реализующий концепцию Callback
     * \param service Ссылка на io_service
     */
    protoclient(Callback &callback, boost::asio::io_service& service)
        : socket_(service),
          buffer_(new char[max_packet_size + header_size]),
          callback_(callback),
          reconnect_timer_(service)
    {
    }

    /*!
     * \brief client Конструктор класса, в котором происходит инициализация сокета и колбека,
     * а также вызывается метод для асинхронного подключения
     * \param callback Ссылка на объект, реализующий концепцию Callback
     * \param service Ссылка на объект, реализующий концепцию Callback
     * \param ep Объект класса endpoint
     */
    protoclient(Callback &callback, boost::asio::io_service& service, boost::asio::ip::tcp::endpoint ep)
        : socket_(service),
          buffer_(new char[header_size + max_packet_size]),
          callback_(callback),
          reconnect_timer_(service)
    {
        async_connect(ep);
    }

    /*!
     * \brief Деструктор
     */
    ~protoclient()
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
        ep_ = ep;
        do_connect(ep);
    }

    /*!
     * \brief Метод для отправки пакета данных
     * \param msg Строка для отправки
     */
    void async_send_message(protocol::Packet::Payload const& payload)
    {
        protocol::Packet packet;
        packet.set_kind(protocol::Packet::Kind::Packet_Kind_Message);
        packet.set_allocated_payload(&payload);
        do_send_packet(packet.SerializeAsString(),packet.kind());
    }

    template<class Request_callback> void async_send_request(protocol::Packet::Payload const& payload, Request_callback& req_callback)
    {
        protocol::Packet packet;
        packet.set_kind(protocol::Packet::Kind::Packet_Kind_Request);
        packet.transaction().set_id(request_id);
        boost::signals2::signal<void(std::string)> on_finished_;
        on_finished_.connect(boost::bind(&Request_callback::on_finished, &req_callback));
        req_resp.emplace(request_id++, on_finished_);
        packet.set_payload(payload);
        do_send_packet(packet.SerializeAsString(),packet.kind());
    }

private:

    /*!
     * \brief Выполнить подключение
     */
    void do_connect(boost::asio::ip::tcp::endpoint ep)
    {
        socket_.async_connect(
            ep, boost::bind(&protoclient::on_connect, this,
                            boost::asio::placeholders::error));
    }

    /*!
     * \brief Послать пакет
     * \param msg Содержимое пакета
     */
    void do_send_packet(const std::string& packet, int kind_)
    {
        // Формируем заголовок
        auto header = reinterpret_cast<packet_header *>(buffer_.get());
        header->packet_size = packet.size();

        // Копируем отправляемую строку в буффер
        copy(packet.begin(), packet.end(), buffer_.get() + header_size);

        // Добавляем асинхронный таск на отправку
        async_write(
            socket_,
            boost::asio::buffer(buffer_.get(), header_size + packet.size()),
            boost::bind(&protoclient::on_packet_sent, this,
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred,
                        kind_));
    }

    /*!
     * \brief Колбек, вызываемый по окончании попытки соединения
     *
     * \param err Ошибка соединения (если есть)
     */
    void on_connect(const error_code& err)
    {
        callback_.on_connected(err);
        if (err)
        {
            reconnect_timer_.expires_from_now(boost::posix_time::milliseconds(proto_reconnect_interval));
            reconnect_timer_.async_wait(boost::bind(&protoclient::do_connect, this, ep_));
        }
    }

    /*!
     * \brief Колбек, вызываемый по окончании отправка пакета
     *
     * \param err Ошибка отправки (если есть)
     * \param bytes Размер отправленного пакета в байтах
     */
    void on_packet_sent(const error_code& err, size_t bytes, int kind_)
    {
        if (boost::asio::error::eof == err || boost::asio::error::connection_reset == err)
        {
            stop();
            do_connect(ep_);
        }
        else
        {
            switch(kind_)
            {
            case protocol::Packet::Kind::Packet_Kind_Message:
                callback_.on_packet_sent(err, bytes);
                break;
            case protocol::Packet::Kind::Packet_Kind_Request:
                callback_.on_packet_sent(err, bytes);
                do_read_header();
                break;
            }
        }
    }

    /*!
     * \brief Асинхронный метод чтения хедера ответа на запрос
     */
    void do_read_header()
    {
        async_read(
            socket_,
            boost::asio::buffer(&packet_header_, header_size),
            boost::asio::transfer_exactly(header_size),
            boost::bind(&protoclient::on_header_read,
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
            boost::asio::buffer(buffer_.get(), packet_size),
            boost::asio::transfer_exactly(packet_size),
            boost::bind(&protoclient::on_packet_read,
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
        if (err)
            return;

        // Закрываем соединение при размере пакета превышающем максимально допустимое
        if (packet_header_.packet_size > max_packet_size)
        {
#ifdef _DEBUG
            std::cout << "connection has been terminated" << "\n";
#endif
            stop();
            return;
        }

        do_read_packet();
    }

    /*!
     * \brief Колбек, вызываемый по окончании чтения пакета
     * \param err Ошибка (если есть)
     * \param bytes Прочитанный размер полученного пакета в байтах
     */
    void on_packet_read(const error_code& err, size_t bytes)
    {
        if (err)
            return;

        // Уведомляем о новом пакете
        on_new_packet(buffer_.get(), bytes);
    }

    void on_new_packet(char const *buffer, size_t nbytes)
    {
        protocol::Packet packet;
        packet.ParseFromString(std::string(buffer, nbytes));

        switch(packet.kind())
        {
        case protocol::Packet::Kind::Packet_Kind_Message:
            callback_.on_new_packet(packet.payload(), bytes);
            break;
        case protocol::Packet::Kind::Packet_Kind_Request:
            break;
        case protocol::Packet::Kind::Packet_Kind_Response:
            req_resp[packet.transaction().id()](packet.payload());
            req_resp.erase(packet.transaction().id()); // Надо ли удалять из map или нет?
            break;
        }
    }

    //! Сокет
    boost::asio::ip::tcp::socket socket_;

    //! Буфер для отправки пакета
    std::unique_ptr<char> buffer_;

    //! Ссылка на объект, предоставляющий callback-функции
    Callback& callback_;

    //! Эндпоинт, используется при повторном подключении
    boost::asio::ip::tcp::endpoint ep_;

    //! Таймер для переподключения
    boost::asio::deadline_timer reconnect_timer_;

    //! Заголовок текущего пакета
    packet_header packet_header_;

    std::map<int, boost::signals2::signal<void(std::string)>> req_resp;

    int request_id = 0;
};

} // namespace link
} // namespace protort
} // namespace alpha

#endif // PROTOCLIENT_H
