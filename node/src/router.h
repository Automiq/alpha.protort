#ifndef ROUTER_H
#define ROUTER_H

#include <iostream>
#include <map>
#include <vector>
#include <boost/bind.hpp>
#include <boost/atomic.hpp>

#include "packet.pb.h"
#include "client.h"
#include "node.h"

// Декларируем функцию, используемую в автотесте
namespace alpha {
namespace protort {
namespace node {
namespace tests {
void test_node_router();
}
}
}
}

namespace alpha {
namespace protort {
namespace node {

using component_shared_ptr = boost::shared_ptr<alpha::protort::components::component>;
using port_id = alpha::protort::components::port_id;

/*!
 * \brief Роутер пакетов
 */
template<class app>
class router : public boost::enable_shared_from_this<router<app>>
{
    friend class node;
    friend class alpha::protort::components::component;
    friend void alpha::protort::node::tests::test_node_router();

    class remote_host
    {
        friend class remote_pair;
    public:
        remote_host(std::string name, boost::shared_ptr<protolink::client<app>> client)
        {
            host_name=name;
            this->client=client;
        }

        virtual ~remote_host()
        {

        }

        virtual void send_message(alpha::protort::protocol::Packet::Payload const &payload)
        {
            client->async_send_message(payload);
        }
    private:
        std::string host_name;
        boost::shared_ptr<protolink::client<app>> client;
    };

    class remote_pair: public remote_host
    {
    public:
        remote_pair(std::string name, boost::shared_ptr<protolink::client<app>> client, std::string name2,
                    boost::shared_ptr<protolink::client<app>> client2): remote_host(name, client)
        {
            host2_name=name2;
            this->client2=client2;
        }

        ~remote_pair()
        {

        }

        //отправляет сообщение мастер ноде
        void send_message(alpha::protort::protocol::Packet::Payload const &payload)
        {
            if(is_master_valid)
            {
                std::cout << "Sending to " << host_name << "message" << std::endl;
                client->async_send_message(payload);
            }
            else
            {
                std::cout << "Sending to " << host2_name << "message" << std::endl;
                client2->async_send_message(payload);
            }
        }

        //формираует запрос бэкап статуса и отправляет мастер ноде
        void request_backup_status()
        {
            alpha::protort::protocol::Packet_Payload status;
            status.mutable_deploy_packet()->set_kind(alpha::protort::protocol::deploy::GetStatus);
            auto callbacks = boost::make_shared<alpha::protort::protolink::request_callbacks>();

            callbacks->on_finished.connect([&](const alpha::protort::protocol::Packet_Payload& packet) {
                on_backup_status_finished(packet.deploy_packet());
            });

            ++timeout;
            if(timeout>3)
            {
                switch_nodes();
            }

            if(is_master_valid)
            {
                std::cout << "Sending to " << host_name << " Backupstatus request" << std::endl;
                client->async_send_request(status, callbacks);
            }
            else
            {
                std::cout << "Sending to " << host2_name << " Backupstatus request" << std::endl;
                client2->async_send_request(status, callbacks);
            }
        }

        //Должел ли он быть public?
        void switch_nodes()
        {
            timeout=0;
            is_master_valid = !is_master_valid;
        }

    private:
        bool is_master_valid;
        int timeout = 0;
        std::string host2_name;
        boost::shared_ptr<protolink::client<app>> client2;

        //Колбек по завершению запроса статуса
        void on_backup_status_finished(const alpha::protort::protocol::deploy::Packet& packet)
        {
            --timeout;
            if(packet.has_error() || !packet.has_response())
            {
                return;
            }
            auto resp = packet.response().status();
            std::cout << "|||BackUpStatus: " << resp.node_info().backup_status() << " of nodename: " << resp.node_name() << "|||" << std::endl;
            if(resp.node_info().backup_status()!=alpha::protort::protocol::backup::BackupStatus::Master)
            {
                switch_nodes();
            }
        }
    };

    //RemotePair test
    boost::shared_ptr<router<node>::remote_pair> sp_rp;
private:
    class component_instance;

    /*!
     * \brief Класс локального маршрута
     * Используется для адресации локального компонента.
     */
    class local_route
    {
    public:
        //! Входной порт адресуемого компонента
        port_id in_port;

        //! Указатель на экземпляр адресуемого компонента
        component_instance * component;
    };

    /*!
     * \brief Класс удаленного маршрута
     * Используется для адресации удаленного компонента.
     */
    class remote_route
    {
    public:
        //! Входной порт адресуемого компонента
        port_id in_port;

        //! Идентификатор компонента
        std::string name;

        //! Указатель на клиентское подключение
        boost::shared_ptr<protolink::client<app>> client;
    };

    /*!
     * \brief Класс маршрутов
     */
    class routes
    {
    public:
        //! Список локальных маршрутов
        std::vector<local_route> local_routes;

        //! Список удаленных маршрутов
        std::vector<remote_route> remote_routes;
    };

    /*!
     * \brief Экземпляр компонента
     * Класс используется для маршрутизации пакетов между компонентами в рамках графа,
     * описывающего структуру приложения. Содержит необходимую информацию о компоненте
     * для его идентификации внутри графа.
     */
    class component_instance
    {
    public:
        //! Указатель на объект компонента
        component_shared_ptr component_;

        //! Идентификтор экземпляра компонента
        std::string name;

        //! Таблица маршрутов
        std::map<port_id, routes> port_to_routes;
    };

public:
    router(boost::asio::io_service& service): service_(service)
    {

    }

    ~router()
    {
#ifdef _DEBUG
        std::cout << "Router::destoooooooyyyeeeeeeedddddddddddddddddddddddd" << std::endl;
#endif
    }

    //! Запускает каждый компонент
    void start()
    {
        started_ = true;
        for (auto & comp : components_) {
            comp.second.component_->start();
        }
    }

    //! Останавливает каждый компонент
    void stop()
    {
        started_ = false;
        for (auto & comp : components_) {
            comp.second.component_->stop();
        }
    }

    //! Удаляет компоненты и клиентов
    void clear()
    {
        components_.clear();
        clients_.clear();
    }

    /*!
     * \brief Отдает входящий пакет на обработку соответствующему компоненту
     * \param component_name Идентификатор компонента
     * \param port Идентификатор входящего порта
     * \param payload Содержимое пакета
     */
    void route(const std::string& component_name, port_id in_port, const std::string& payload)
    {
        in_packets_++;

        if (!started_)
        {
#ifdef _DEBUG
            boost::mutex::scoped_lock lock(cout_mutex);
            std::cout << "route packet at stopped router" << std::endl;
#endif
            return;
        }

        auto it = components_.find(component_name);
        if (it != components_.end())
        {
#ifdef _DEBUG
            boost::mutex::scoped_lock lock(cout_mutex);
            std::cout << "route::post(do_process payload)" << std::endl;
#endif
            service_.post(boost::bind(&protort::components::component::do_process,
                                     it->second.component_,
                                     in_port,
                                     payload));
        }
    }
    int i=0;
    /*!
     * \brief Рассылает выходные данные компонента по маршрутам
     * \param comp_inst Компонент
     * \param outputs Выходные данные компонента
     */
    void do_route(void *comp_inst,
                  const std::vector<alpha::protort::components::output>& outputs)
    {
        assert(comp_inst != nullptr);

        if (!started_)
        {
#ifdef _DEBUG
            boost::mutex::scoped_lock lock(cout_mutex);
            std::cout << "do_route at stopped router" << std::endl;
#endif
            return;
        }

        component_instance* this_component = static_cast<component_instance*>(comp_inst);

        for (auto &output : outputs)
        {
            for (auto const &out_port : output.ports)
            {
                routes& port_routes = this_component->port_to_routes[out_port];

                // Рассылаем пакеты по локальным маршрутам
                for (auto &local_route : port_routes.local_routes)
                {
#ifdef _DEBUG
                    boost::mutex::scoped_lock lock(cout_mutex);
                    std::cout << "using do_route: \nfrom comp " << this_component->name
                              << " out port " << out_port << std::endl;
                    std::cout << "to comp " << local_route.component->name
                              << " in port " << local_route.in_port << std::endl;
#endif
                    service_.post(boost::bind(&protort::components::component::do_process,
                                             local_route.component->component_,
                                             local_route.in_port,
                                             output.payload));
                }

                // Формируем и рассылаем пакеты по удаленным маршрутам
                for (auto &remote_route : port_routes.remote_routes)
                {
                    alpha::protort::protocol::Packet::Payload payload;
                    auto packet = payload.mutable_communication_packet();

                    // out endpoint
                    packet->mutable_source()->set_port(static_cast<uint32_t>(out_port));
                    packet->mutable_source()->set_name(this_component->name);

                    // in endpoint
                    packet->mutable_destination()->set_port(static_cast<uint32_t>(remote_route.in_port));
                    packet->mutable_destination()->set_name(remote_route.name);

                    // payload
                    packet->set_payload(output.payload);

                    //запрос о статусе. Если ответил не мастер то отправка произойдет на другую ноду.
                    if(i<10)
                    {
                        i++;
                    }
                    else
                    {
                        sp_rp->request_backup_status();
                        i=0;
                    }
                    sp_rp->send_message(payload);
                    //remote_route.client->async_send_message(payload); //старый метод


                    out_bytes_ += payload.ByteSize();
                    out_packets_++;
#ifdef _DEBUG
                    boost::mutex::scoped_lock lock(cout_mutex);
                    std::cout << "Sending packet to " << remote_route.name << std::endl;
#endif
                }
            }
        }
    }

    //! Выдает ссылку на io_service роутера
    boost::asio::io_service& get_service()
    {
        return service_;
    }

private:
    //! Таблица компонентов
    std::map<std::string, component_instance> components_;

    //! Таблица удаленных получателей пакетов
    std::map<std::string, boost::shared_ptr<protolink::client<app>>> clients_;

    //! I/O сервис
    boost::asio::io_service& service_;

    //! Статус роутера
    boost::atomic_bool started_{false};

    //! Статистика по принятым байтам
    boost::atomic_uint32_t in_bytes_{0};

    //! Статистика по отправленным байтам
    boost::atomic_uint32_t out_bytes_{0};

    //! Статистика по принятым пакетам
    boost::atomic_uint32_t in_packets_{0};

    //! Статистика по отправленным пакетам
    boost::atomic_uint32_t out_packets_{0};

#ifdef _DEBUG
    boost::mutex cout_mutex;
#endif
};

} // namespae node
} // namespae protort
} // namespae alpha


#endif // ROUTER_H
