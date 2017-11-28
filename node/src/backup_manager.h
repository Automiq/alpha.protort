#ifndef BACKUP_MANAGER_H
#define BACKUP_MANAGER_H

#include <boost/date_time/posix_time/posix_time.hpp>

#include "node.h"
#include "router.h"

const uint32_t timeout_const=10;


namespace alpha {
namespace protort {
namespace node {


enum class Node_status {master=1 , slave};

using port_id=uint32_t;

struct address
{
    /*!
     * \brief IP-адрес или hostname узла
     */
    std::string ip_address;

    /*!
     * \brief Порт
     */
    port_id port;

    /*!
     * \brief Порт сервера конфигурации (по умолчанию =100)
     */
    port_id config_port;
};


class Life_master: public boost::enable_shared_from_this<Life_master>
{
    using port_id=uint32_t;
    using client_t=alpha::protort::protolink::client<Backup_manager>;
    using client_ptr=boost::shared_ptr<client_t>;
public:

    Life_master(boost::asio::io_service& service,
                uint32_t interval ,
                client_ptr client):
        IO_service_(service),
        timer_(service),
        interval_(interval),
        client_(client),
        count_timeout_(0)
    {
    }

    ~Life_master(){}

    void start_check(Node_status node_status){

        if(node_status==Node_status::slave){

            IO_service_.post(boost::bind(&Life_master::dispatch_on_master ,
                                            boost::static_pointer_cast<Life_master>(this->shared_from_this())));

        }
        else{

            return;

        }

    }

    void operator=(Life_master LM){

        IO_service_=LM.IO_service_;
        timer_=LM.timer_;
        interval_=LM.interval_;

    }

private:

    void timeout(const alpha::protort::protocol::Packet_Payload& packet){
        count_timeout_--;
    }

    void master_check(){
        timer_.expires_from_now(boost::posix_time::milliseconds(this->interval_));
        timer_.async_wait(boost::bind(&Life_master::dispatch_on_master ,
                          boost::static_pointer_cast<Life_master>(this->shared_from_this())));
    }

    bool dispatch_on_master(){
            //отправка пакета keepalife
            //если ответ не пришел то возвращаем false
            //реализовать
        count_timeout_++;
        alpha::protort::protocol::Packet_Payload packet;
        auto callback = boost::make_shared<alpha::protort::protolink::request_callbacks>();
        callback->on_finished.connect([&](const alpha::protort::protocol::Packet_Payload& packet){
            timeout(packet.deploy_packet() );
        });
        client_->async_send_request(packet , callback);

        IO_service_.post(boost::bind(&Life_master::master_check ,
                                           boost::static_pointer_cast<Life_master>(this->shared_from_this())));
    }
    //IO сервис ноды
    boost::asio::io_service &IO_service_;
    //бустовский дедлайн таймер
    boost::asio::deadline_timer timer_;
    //переод через который будет производится запрос на мастер со слейва
    uint32_t interval_;
    //клиент для связи с парной нрдой
    client_ptr client_;
    uint32_t count_timeout_;
};





class Backup_manager: public boost::enable_shared_from_this<Backup_manager>
{
    using client_t=alpha::protort::protolink::client<Backup_manager>;
    using client_ptr=boost::shared_ptr<client_t>;
public:
    Backup_manager(std::string &addres_pair_node ,
                   port_id &pair_node_port ,
                   boost::asio::io_service& service,
                   Node_status node_status
                 ):
        service__(service),
        node_status_(node_status)
    {
        if(node_status_==Node_status::slave){
            life_master_=boost::make_shared<Life_master>(service, 500);
            start_keepalife();
        }
        address_.port=pair_node_port;
        address_.ip_address=addres_pair_node;
        client_=boost::make_shared<client_t>(this->shared_from_this() , service__);
        boost::asio::ip::tcp::endpoint ep(
                    boost::asio::ip::address::from_string(address_.ip_address , address_.port)
                    );
        client_->async_connect(ep);
    }

    ~Backup_manager(){

    }

    Node_status backup_status(){

        return  node_status_;

    }

    bool backup_transition(){
        //проверка пары на жизнь если он жив то
        //отправка на пару запроса на backup_nransition

        if(node_status_==Node_status::slave){

            switch_status();

        }
        else{

            life_master_=boost::make_shared<Life_master>(service__ , 500);

        }

    }


private:

    void start_keepalife(){

        life_master_.get()->start_check(node_status_);
        //    backup_transition();

    }

    void switch_status(){

        node_status_=(Node_status::slave==node_status_)?Node_status::master : Node_status::slave;

    }

    //io сервис узла
    boost::asio::io_service& service__;
    //статус узла
    Node_status node_status_;
    //объект для проверки жизни мастера
    boost::shared_ptr<Life_master> life_master_;
    //
    client_ptr client_;
    //
    address address_;
};




        }
    }
}
#endif // BACKUP_MANAGER_H
