#ifndef BACKUP_MANAGER_H
#define BACKUP_MANAGER_H

#include <boost/date_time/posix_time/posix_time.hpp>

#include "node.h"
#include "router.h"

namespace alpha {
namespace protort {
namespace node {


enum class Node_status {master=1 , slave};

class Life_master: public boost::enable_shared_from_this<Backup_manager>
{
public:

    Life_master(boost::asio::io_service& service, boost::shared_ptr<router<node>> rout , uint32_t interval):
        IO_service_(service),
        rout_to_backup_(rout),
        timer_(service),
        interval_(interval)
    {}

    ~Life_master(){}

    start_check(Node_status node_status){

        if(node_status==Node_status::slave){

            IO_service_.post(boost::bind(&Backup_manager::dispatch ,
                                            boost::static_pointer_cast<Life_master>(this->shared_from_this())));

        }
        else{

            return;

        }

    }

private:
    void master_check(){
        timer_.expires_from_now(boost::posix_time::milliseconds(this->interval_));
        timer_.async_wait(boost::bind(&Backup_manager::dispatch ,
                          boost::static_pointer_cast<Life_master>(this->shared_from_this())));
    }

    bool dispatch_on_master(){

        boost::shared_ptr<router<node>> rout_master=rout_to_backup_.lock();
        if(rout_master){
            //отправка пакета keepalife
            //если ответ не пришел то возвращаем false
            //реализовать
            rout_master->get_service().post(boost::bind());

        }
        IO_service_.post(boost::bind(&Backup_manager::master_check ,
                                           boost::static_pointer_cast<Life_master>(this->shared_from_this())));
    }
    //IO сервис ноды
    boost::asio::io_service &IO_service_;
    //бустовский дедлайн таймер
    boost::asio::deadline_timer timer_;
    //переод через который будет производится запрос на мастер со слейва
    uint32_t interval_;
    //роутер до парного узла для общения с ним
    boost::weak_ptr<router<node>> rout_to_backup_;

};

class Backup_manager: public boost::enable_shared_from_this<Backup_manager>
{
public:
    Backup_manager(std::string &addres_pair_node ,
                   uint32_t &pair_node_port ,
                   boost::asio::io_service& service,
                   boost::shared_ptr<router<node>> rout_to_backup,
                   Node_status node_status
                 ):
        addres_pair_node_(addres_pair_node),
        pair_node_port_(pair_node_port),
        service__(service),
        node_status_(node_status),
        rout_to_backup_(rout_to_backup)
    {
        if(node_status_==Node_status::slave){
            life_master_(service , rout_to_backup , 500);
            start_keepalife();
        }

    }

    start_keepalife(){

        if(this->life_master_.start_check(node_status_)){

            backup_transition();

        }

    }

    ~Backup_manager(){

    }

    bool backup_transition(){
        //проверка пары на жизнь если он жив то
        //отправка на пару запроса на backup_nransition

        if(node_status_==Node_status::slave){

            delete life_master_;
            life_master_=nullptr;
            switch_status();

        }
        else{

            life_master_(service__ , rout_to_backup_ , 500);

        }

    }


private:

    void switch_status(){
        node_status_=(Node_status::slave==node_status_)?Node_status::master : Node_status::slave;
    }

    //адрес парного узла
    std::string addres_pair_node_;
    //порт парного узла
    uint32_t pair_node_port_;
    //io сервис узла
    boost::asio::io_service& service__;
    //статус узла
    Node_status node_status_;
    //роутер до парного узла для общения с ним
    boost::weak_ptr<router<node>> rout_to_backup_;
    //объект для проверки жизни мастера
    Life_master life_master_;

};




        }
    }
}
#endif // BACKUP_MANAGER_H
