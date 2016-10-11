#ifndef LINK_CLIENT_H
#define LINK_CLIENT_H

#include <boost/bind.hpp>
#include <boost/asio.hpp>

using namespace boost::asio;


template<typename LinkClientCallback> class link_client
{
    using error_code = boost::system::error_code;

    ip::tcp::socket sock_;
    enum { max_msg = 1024 };
    char read_buffer_[max_msg];
    char write_buffer_[max_msg];
    bool started_;
    LinkClientCallback& callback_;

    void on_connect(const error_code & err)
    {
        std::cout << "connected link\n";
        if (!err)
            callback_.on_connected();
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
        if (err)
            stop();
        if (!started_)
            return;
        std::string msg(read_buffer_, bytes);
        std::cout << msg << "\n";
    }

    void on_write(const error_code & err, size_t bytes)
    {
        do_read();
    }
    void do_read()
    {
        async_read(sock_, buffer(read_buffer_),boost::bind(&link_client<LinkClientCallback>::read_complete,this,_1,_2), boost::bind(&link_client<LinkClientCallback>::on_read,this,_1,_2));
    }
    void do_write(const std::string& msg)
    {
        if (!started_)
            return;
        copy(msg.begin(), msg.end(), write_buffer_);
        sock_.async_write_some( buffer(write_buffer_, msg.size()),boost::bind(&link_client<LinkClientCallback>::on_write,this,_1,_2));
    }
    size_t read_complete(const error_code & err, size_t bytes)
    {
        if (err)
            return 0;
        bool found = std::find(read_buffer_, read_buffer_ + bytes, '\n') < read_buffer_ + bytes;
        return found ? 0 : 1;
    }
public:
    link_client(LinkClientCallback &callback, boost::asio::io_service& service):sock_(service),started_(true),callback_(callback)
    {

    }
    link_client(LinkClientCallback &callback, boost::asio::io_service& service, ip::tcp::endpoint ep):sock_(service),started_(true),callback_(callback)
    {
        sock_.async_connect(ep, boost::bind(&link_client<LinkClientCallback>::on_connect,this,_1));
    }
    void connect_async(ip::tcp::endpoint ep)
    {
        sock_.async_connect(ep, boost::bind(&link_client<LinkClientCallback>::on_connect, this, _1));
    }
    void send_async(std::string const& msg)
    {
        do_write(msg);
    }
    ~link_client()
    {
        stop();
    }
};

#endif // LINK_CLIENT_H
