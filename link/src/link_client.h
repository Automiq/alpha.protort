#ifndef LINK_CLIENT_H
#define LINK_CLIENT_H

#include <boost/bind.hpp>
#include <boost/asio.hpp>

using namespace boost::asio;


template<class LinkClientCallback> class link_client
{
    using error_code = boost::system::error_code;

    ip::tcp::socket sock_;
    enum { max_msg = 100000 };
    char read_buffer_[max_msg];
    char write_buffer_[max_msg];
    bool started_;
    LinkClientCallback& callback_;
    //deadline_timer timer;
    void on_connect(const error_code & err)
    {
        if (!err)
            callback_.on_connected(this);
        else
            stop();
    }

    void stop()
    {
        std::cout << "connection has stopped\n";
        started_ = false;
        sock_.close();
    }
    void on_read(const error_code & err, size_t bytes)
    {
        std::cout << "on_read " << err << "\n";
        if (err)
            stop();
        if (!started_)
            return;
        std::cout << "on_read worked" << "\n";
        std::string msg(read_buffer_, bytes);
        std::cout << "client message received: " << msg << "\n";
    }

    void on_write(const error_code & err, size_t bytes)
    {
        std::cout << "on_write " << err << " and " << bytes << "\n";
        do_read();
    }
    void do_read()
    {
        async_read(sock_, buffer(read_buffer_),boost::bind(&link_client::on_read,this,_1,_2));
    }
    void do_write(const std::string& msg)
    {
        if (!started_)
            return;
        std::cout << "Sended from client: " << msg << "\n";
        copy(msg.begin(), msg.end(), write_buffer_);
        sock_.async_write_some(buffer(write_buffer_, msg.size()),boost::bind(&link_client::on_write,this,_1,_2));
    }
public:
    link_client(LinkClientCallback &callback, boost::asio::io_service& service):sock_(service),started_(true),callback_(callback)
    {

    }
    link_client(LinkClientCallback &callback, boost::asio::io_service& service, ip::tcp::endpoint ep):sock_(service),started_(true),callback_(callback)
    {
        connect_async(ep);
    }
    void connect_async(ip::tcp::endpoint ep)
    {
        sock_.async_connect(ep, boost::bind(&link_client::on_connect, this, boost::asio::placeholders::error));
    }
    void send_async(std::string const& msg)
    {
        //timer.expires_from_now(boost::posix_time::millisec(1000));
        //timer.async_wait(boost::bind(&link_client::do_write,this,msg));
        do_write(msg);
    }
    ~link_client()
    {
        stop();
    }
};

#endif // LINK_CLIENT_H
