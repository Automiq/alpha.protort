#ifndef LOGI_H
#define LOGI_H

#include <cstring>
#include <fstream>
#include <iostream>
#include <ctime>
#include <sstream>
#include <string>
#include <boost/shared_ptr.hpp>


class logger{
public:
    logger(){}
    virtual ~logger(){}
    virtual void log(const std::string &type ,const std::string &message){}
protected:
        std::string build_message(const std::string &type_message,const std::string &message){
        time_t ttime=time(NULL);
        tm* this_time=localtime(&ttime);
        std::stringstream ss_message;
        ss_message<<(int)(this_time->tm_mday)<<'/'<<(int)(this_time->tm_mon)<<'/'<<1900+(int)(this_time->tm_year)
                 <<' '<<(int)(this_time->tm_hour)<<':'
                 <<(int)(this_time->tm_min)<<':'
                 <<(int)(this_time->tm_sec)<<'-'<<type_message<<':'<<message<<'\n';
        delete this_time;
        return  ss_message.str();
    }
private:
    friend class logger_component;
    friend class logger_node;
};

class logger_file: public logger{
public:
    logger_file(std::string &file_name){
        file_name+=".txt";
        file.open(file_name);
    }

    virtual void log(const std::string  &type,const std::string &message) override {
        std::string message_=build_message(type , message);
        file<<message_<<std::endl;
    }

    virtual ~logger_file() override{
        file.close();
    }

private:
    std::ofstream file;
};

class logger_stdout: public logger{
public:
    logger_stdout(){}
    virtual ~logger_stdout() override{}
    virtual void log(const std::string &type ,const std::string &message) override{
        std::string message_=build_message(type , message);
        std::cout<<message_<<std::endl;
    }
};

class logger_component : public logger{
public:
    logger_component(const boost::shared_ptr<logger> &log,  std::string &comp_name ,  std::string &comp_type){
        templ=comp_name+':'+comp_type;
        log_=log;
    }
    virtual ~logger_component() override{

    }
    virtual void log(const std::string &type ,const  std::string &message) override{
        std::stringstream ss_message;
        ss_message<<templ<<':'<<message;
        log_->log(type, ss_message.str());
    }
private:
    std::string templ;
    boost::shared_ptr<logger> log_;
};

class logger_node : public logger{
public:
    logger_node(const boost::shared_ptr<logger> &log ,const std::string &node_name){
        templ=node_name;
        log_=log;
    }
    virtual ~logger_node() override{

    }
    virtual void log(const std::string &type , const std::string &message) override{

        std::stringstream ss_message;
        ss_message<<templ<<':'<<message;
        log_->log(type , ss_message.str());

    }
private:
    std::string templ;
    boost::shared_ptr<logger> log_;
};


/*
 * [time]:[node_name]/[component]:[type]:[message];
*/

#endif // LOGI_H
