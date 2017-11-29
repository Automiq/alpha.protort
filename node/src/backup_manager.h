#ifndef BACKUP_MANAGER_H
#define BACKUP_MANAGER_H



#include <boost/date_time/posix_time/posix_time.hpp>


#include "node.h"



//время тайм аута для keepalife в секундах
const uint32_t timeout_time=10;

namespace alpha {
namespace protort {
namespace node {

enum Node_status {master=1 , slave};

class Backup_manager;

class master_manager: public boost::enable_shared_from_this<master_manager>
{
    friend class Backup_manager;
    using port_id=uint32_t;
    using client_t=alpha::protort::protolink::client<node>;
    using client_ptr=boost::shared_ptr<client_t>;

public:

    master_manager(boost::asio::io_service& service,
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
    }

    ~master_manager(){
       // signal_->on_timeout.realised();
       // спросить как лучше отвязать сигналы
        timer_.cancel();
    }

    void start_check(){
            IO_service_.post(boost::bind(&master_manager::dispatch_on_master ,
                                            boost::static_pointer_cast<master_manager>(this->shared_from_this())));
    }

    boost::shared_ptr<alpha::protort::protolink::request_callbacks> get_signal(){
        return signal_;
    }

private:

    void master_is_life(const alpha::protort::protocol::backup::Packet& packet){
        count_timeout_--;
    }

    void master_check(){
        timer_.expires_from_now(boost::posix_time::milliseconds(this->interval_));
        timer_.async_wait(boost::bind(&master_manager::dispatch_on_master ,
                          boost::static_pointer_cast<master_manager>(this->shared_from_this())));
    }

    bool dispatch_on_master(){
            //отправка пакета keepalife
            //если ответ не пришел то возвращаем false
            //реализовать

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

        IO_service_.post(boost::bind(&master_manager::master_check ,
                                           boost::static_pointer_cast<master_manager>(this->shared_from_this())));
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
    uint32_t count_timeout_;
    //сигнал
    boost::shared_ptr<alpha::protort::protolink::request_callbacks> signal_;
};


class Backup_manager: public boost::enable_shared_from_this<Backup_manager>
{
    using client_t=alpha::protort::protolink::client<node>;
    using client_ptr=boost::shared_ptr<client_t>;
    using port_id=uint32_t;
public:
    Backup_manager(std::string &addres_pair_node ,
                   port_id &pair_node_port ,
                   boost::asio::io_service& service,
                   Node_status node_status,
                   client_ptr client
                 ):
        service__(service),
        node_status_(node_status),
        client_(client)
    {
    }

    ~Backup_manager(){

    }
// спросить можноли так делать в данном случае
    alpha::protort::protocol::backup::BackupStatus backup_status(){
        return (alpha::protort::protocol::backup::BackupStatus)node_status_;
    }



   void backup_is_life(const alpha::protort::protocol::backup::Packet& packet){
        switch_status();
        start_keepalife();
      //  protocol_payload responce;
      //  responce.mutable_backup_packet()->set_kind(protocol::backup::SwitchResponse);
      //  return responce;
    }

    void backup_transition(){
        //проверка пары на жизнь если он жив то
        //отправка на пару запроса на backup_nransition
        if(node_status_==Node_status::slave){
            switch_status();
            life_master_.get_deleter();//спросить
        //    return true;
        }
        else{
            alpha::protort::protocol::Packet_Payload packet;
            packet.mutable_backup_packet()->set_kind(alpha::protort::protocol::backup::Switch);
            auto callback = boost::make_shared<alpha::protort::protolink::request_callbacks>();
            callback->on_finished.connect([&](const alpha::protort::protocol::Packet_Payload& packet){
                backup_is_life(packet.backup_packet());
            });
            client_->async_send_request(packet , callback);
            //life_master_=boost::make_shared<Life_master>(service__ , 500);
        }

    }


    void start_keepalife(){
        if(node_status_==Node_status::slave){
            life_master_=std::make_unique<master_manager>(service__ , 500 , client_);
            life_master_->get_signal()->on_timeout.connect([&](){
                backup_transition();
            });
            life_master_.get()->start_check();
            //    backup_transition();
        }
    }

private:

    void switch_status(){

        node_status_=(Node_status::slave==node_status_)?Node_status::master : Node_status::slave;

    }

    void loss_of_master(){
        this->backup_transition();
    }

    //io сервис узла
    boost::asio::io_service& service__;
    //статус узла
    Node_status node_status_;
    //объект для проверки жизни мастера
    std::unique_ptr<alpha::protort::node::master_manager> life_master_;
    //
    client_ptr client_;
};




        }
    }
}
#endif // BACKUP_MANAGER_H
