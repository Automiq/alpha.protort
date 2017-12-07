#ifndef BACKUP_MANAGER_H
#define BACKUP_MANAGER_H



#include <boost/date_time/posix_time/posix_time.hpp>
#include "node.h"



//время тайм аута для keepalife в секундах
const uint32_t timeout_time=1;

namespace alpha {
namespace protort {
namespace node {

enum Node_status {master=1 , slave};

class Backup_manager;

class master_monitor: public boost::enable_shared_from_this<master_monitor>
{
    friend class Backup_manager;
    using port_id=uint32_t;
    using client_t=alpha::protort::protolink::client<node>;
    using client_ptr=boost::shared_ptr<client_t>;
    using protocol_payload = protocol::Packet::Payload;

public:

    master_monitor(boost::asio::io_service& service,
                uint32_t interval,
                client_ptr client
                ):
        IO_service_(service),
        timer_(service),
        interval_(interval),
        client_(client),
        count_timeout_(0)
    {
        signal_=boost::make_shared<alpha::protort::protolink::request_callbacks>();
        flag=true;
    }

    ~master_monitor(){
       // signal_->on_timeout.realised();
       // спросить как лучше отвязать сигналы
    }
    //начало мониторинга мастера
    void start_check(){
            IO_service_.post(boost::bind(&master_monitor::dispatch_on_master ,
                                            boost::static_pointer_cast<master_monitor>(this->shared_from_this())));
    }

    void stop(){
        flag=false;
        timer_.cancel();
    }
    //вызывает сигнал
    boost::shared_ptr<alpha::protort::protolink::request_callbacks> get_signal(){
        return signal_;
    }

private:

    void master_is_life(const alpha::protort::protocol::backup::Packet& packet){
        count_timeout_--;
    }
    //таймер
    void master_check(){
        timer_.expires_from_now(boost::posix_time::milliseconds(this->interval_));
        timer_.async_wait(boost::bind(&master_monitor::dispatch_on_master ,
                          boost::static_pointer_cast<master_monitor>(this->shared_from_this())));
    }
    //отправка запроса на мастер и в случае смерти мастера вызывает backup_transition
    void dispatch_on_master(){
            //отправка пакета keepalife
            //если ответ не пришел то возвращаем false
            //реализовать
        if(flag==false){
            count_timeout_=-timeout_time;
            return;
        }
        if((count_timeout_*interval_)/1000>=timeout_time){
            //надо выполнить backuptransition
            signal_->on_timeout();
        }
        count_timeout_++;
        alpha::protort::protocol::Packet_Payload packet;
        packet.mutable_backup_packet()->set_kind(alpha::protort::protocol::backup::KeepAlive);
        auto callback = boost::make_shared<alpha::protort::protolink::request_callbacks>();
        callback->on_finished.connect([&](const alpha::protort::protocol::Packet_Payload& packet){
            master_is_life(packet.backup_packet());
        });
        client_->async_send_request(packet , callback);
        std::cout<<"keepalife\n";

        IO_service_.post(boost::bind(&master_monitor::master_check ,
                                           boost::static_pointer_cast<master_monitor>(this->shared_from_this())));
    }
    //IO сервис ноды
    boost::asio::io_service &IO_service_;
    //бустовский дедлайн таймер
    boost::asio::deadline_timer timer_;
    //переод через который будет производится запрос на мастер со слейва
    uint32_t interval_;
    //клиент для связи с парной нрдой
    client_ptr client_;
    //количество пакетов от которых не пришел ответ
    boost::atomic<uint32_t> count_timeout_;
    //сигнал для timeout
    boost::shared_ptr<alpha::protort::protolink::request_callbacks> signal_;
    //флаг на работу монитора
    bool flag;
};


class Backup_manager: public boost::enable_shared_from_this<Backup_manager>
{
    using client_t=alpha::protort::protolink::client<node>;
    using client_ptr=boost::shared_ptr<client_t>;
    using port_id=uint32_t;
    using protocol_payload = protocol::Packet::Payload;
public:
    Backup_manager(boost::asio::io_service& service,
                   Node_status node_status,
                   client_ptr client,
                   boost::shared_ptr<router<node> > router
                 ):
        service__(service),
        node_status_(node_status),
        client_(client),
        router_(router)
    {
    }

    ~Backup_manager(){

    }


    //возвращает текущий статус узла
    alpha::protort::protocol::backup::BackupStatus backup_status(){
        return static_cast<protocol::backup::BackupStatus>(node_status_);
    }

    //выполняется резервный переход
    void backup_transition(){
        //проверка пары на жизнь если он жив то
        //отправка на пару запроса на backup_nransition
        if(node_status_==Node_status::master){
            alpha::protort::protocol::Packet_Payload packet;
            packet.mutable_backup_packet()->set_kind(alpha::protort::protocol::backup::Switch);
            auto callback = boost::make_shared<alpha::protort::protolink::request_callbacks>();
            callback->on_finished.connect([&](const alpha::protort::protocol::Packet_Payload& packet){
                backup_is_life(packet.backup_packet());
            });
            client_->async_send_request(packet , callback);
            std::cout<<"backup_transition master\n";
        }
        else{
            router_->activate();
            switch_status();
            if(master_monitor_){
                master_monitor_->stop();
                master_monitor_.reset();
            }
             std::cout<<"backup_transition slave\n";
        }

    }
    /*!
     * \brief Метод формирует ответ на запрос keepalive
     */
    protocol_payload keepalive_response(){
        protocol_payload response;
        protocol::backup::Packet *response_packet = response.mutable_backup_packet();

        response_packet->set_kind(protocol::backup::PacketType::KeepAlive);

        return response;
    }

    protocol_payload backup_status_response()
    {
        protocol_payload backup_response;
        protocol::backup::Packet* backup_response_packet = backup_response.mutable_backup_packet();

        backup_response_packet->set_kind(protocol::backup::PacketType::GetStatus);
        backup_response_packet->mutable_response()->mutable_status()->set_backup_status(backup_status());

        return backup_response;
    }

    //запускает процес проверки мастера на работоспособность
    void start_keepalife(){
        if(node_status_==Node_status::slave){
            master_monitor_=boost::make_shared<master_monitor>(service__ , 500 , client_);
            master_monitor_->get_signal()->on_timeout.connect([&](){
                backup_transition();
            });
            master_monitor_->start_check();
        }
    }

private:
    void backup_is_life(const alpha::protort::protocol::backup::Packet& packet){
        router_->deactivate();
        switch_status();
        start_keepalife();
    }
    //меняет node_status текущей компоненты на противоположный
    void switch_status(){
        node_status_=(Node_status::slave==node_status_)?Node_status::master : Node_status::slave;
    }

    //io сервис узла
    boost::asio::io_service& service__;
    //статус узла
    Node_status node_status_;
    //объект для проверки жизни мастера
    boost::shared_ptr<alpha::protort::node::master_monitor> master_monitor_;
    //клиент для работы с парной нодой
    client_ptr client_;
    //роутер
    boost::shared_ptr<router<node>> router_;
};




        }
    }
}
#endif // BACKUP_MANAGER_H
