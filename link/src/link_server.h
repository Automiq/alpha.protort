#ifndef LINK_SERVER_H
#define LINK_SERVER_H

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
using namespace boost::asio;

template<class LinkServerCallback> class connection : public boost::enable_shared_from_this<connection<LinkServerCallback>>, boost::noncopyable
{
public:
    using error_code = boost::system::error_code;
    using ptr = boost::shared_ptr<connection<LinkServerCallback>>;
private:
    connection(LinkServerCallback& callback,io_service& service):sock_(service),started_(false),callback_(callback)
    {
        std::cout << "server constructor" << "\n";
    }
    void on_read(const error_code & err, size_t bytes)
    {
        std::cout << "server on_read " << err << "\n";
        if ( !err)
        {
            std::string message(read_buffer_,bytes);
            std::cout << "server message number " << count_stop << " :" << message << "\n";
            if(count_stop == 1)
                async_read(sock_, buffer(read_buffer_),boost::asio::transfer_exactly(5), boost::bind(&connection::on_read, shared_from_this(),_1,_2));
            if(count_stop == 2)
            {
                callback_.on_new_packet(read_buffer_, 5);
                //do_write("Go ahead");
                stop();
            }
            count_stop++;
        }
    }

    void on_write(const error_code & err, size_t bytes) {
        //do_read();
    }
    void do_read() {
        std::cout << "server do_read" << "\n";
        async_read(sock_, buffer(read_buffer_),boost::asio::transfer_exactly(4), boost::bind(&connection::on_read, shared_from_this(),_1,_2));
    }
    void do_write(const std::string & msg) {
        std::copy(msg.begin(), msg.end(), write_buffer_);
        sock_.async_write_some(buffer(write_buffer_, msg.size()),boost::bind(&connection::on_write, shared_from_this(),_1,_2));
        std::cout << "Sended from server: " << msg << "\n";
    }
    ip::tcp::socket sock_;
    enum { max_msg = 1024 };
    char read_buffer_[max_msg];
    char write_buffer_[max_msg];
    bool started_;
    int count_stop = 1;
    LinkServerCallback& callback_;

public:

    void start()
    {
        std::cout << "start" << "\n";
        started_ = true;
        do_read();
    }
    static ptr new_(LinkServerCallback& callback,io_service& service)
    {
        ptr new_(new connection(callback,service));
        return new_;
    }
    void stop()
    {
        std::cout << "server connection has been stopped\n";
        if ( !started_)
            return;
        started_ = false;
        sock_.close();
    }
    ip::tcp::socket & sock()
    {
        return sock_;
    }
    ~connection()
    {
        stop();
    }
};

template<class LinkServerCallback> class link_server
{
    using ptr = boost::shared_ptr<connection<LinkServerCallback>>;
    ip::tcp::acceptor acceptor_;
    ptr client;
    LinkServerCallback& callback_;
    void handle_accept(ptr client, const boost::system::error_code & err)
    {
        client->start();
        callback_.on_new_connection();
        ptr new_client = connection<LinkServerCallback>::new_(callback_,acceptor_.get_io_service());
        acceptor_.async_accept(new_client->sock(), boost::bind(&link_server::handle_accept,this,new_client,boost::asio::placeholders::error));
    }
public:
    link_server (LinkServerCallback& callback,io_service& service):acceptor_(service,ip::tcp::endpoint(ip::tcp::v4(), 100)),callback_(callback)
    {
        client = connection<LinkServerCallback>::new_(callback_,service);
    }
    link_server (LinkServerCallback& callback,io_service& service,ip::tcp::endpoint ep):acceptor_(serveice,ep),callback_(callback)
    {
        client = connection<LinkServerCallback>::new_(callback_,service);
    }
    void listen()
    {
        acceptor_.async_accept(client->sock(), boost::bind(&link_server::handle_accept,this,client,boost::asio::placeholders::error));
    }
};

#endif // LINK_SERVER_H
