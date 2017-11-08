#ifndef IOSWRAPPER_H
#define IOSWRAPPER_H
#include <boost/asio.hpp>
#include <boost/atomic.hpp>
#include <boost/thread/mutex.hpp>

class IOSWrapper
{
public:
    IOSWrapper(){

    }

//    IOSWrapper(const IOSWrapper& w)
//    {
//        this->max_queue_size=w.max_queue_size;
//        this->queue=w.queue;
//        this->service_=w.service_;
//        this->queue_mutex_=w.queue_mutex_;
//    }
    ~IOSWrapper(){

    }

    void set_service(boost::asio::io_service& service)
    {
        service_ = &service;
    }

    int getQueue(){
        return queue;
    }

    void incQueue()
    {
        ++queue;
    }

    void decQueue()
    {
        --queue;
        std::cout << "!!!!! Queue size is:" << queue << std::endl;
    }


    template<typename CompletionHandler>
    void post(CompletionHandler foo)
    {
        PostObjWrapper<CompletionHandler> o{foo, this};
        service_->post(o);
    }


private:
    boost::asio::io_service *service_;
    boost::atomic_int queue {0};
    int max_queue_size = 1000;
    boost::mutex queue_mutex_;
};

template<typename CompletionHandler>
class PostObjWrapper
{
public:
    PostObjWrapper(CompletionHandler &foo, IOSWrapper *wrp)
        :foo_(foo),wrp_(wrp){
        wrp_->incQueue();
    }

    PostObjWrapper (const PostObjWrapper& other)
        :foo_(other.foo_), wrp_(other.wrp_)
    {
        wrp_->incQueue();
    }

    void operator()()
    {
       foo_();
    }
    ~PostObjWrapper(){
        wrp_->decQueue();
    }
private:
    CompletionHandler foo_;
    IOSWrapper *wrp_;
};

#endif // IOSWRAPPER_H
