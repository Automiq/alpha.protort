#ifndef LOGI_H
#define LOGI_H

#include <cstring>
#include <fstream>
#include <iostream>
#include <ctime>
#include <sstream>
#include <string>
#include <boost/shared_ptr.hpp>

 /*
  *
  *    string create_message(string node_name="", string comp_name="", string message, string type){
        time_t time_logging=time(NULL);
        stringstream ss_logging;
        time_t t=time_logging-time_start;
        ss_logging<<t<<':'<<node_name<<'/'<<comp_name<<':'<<type<<':'<<message;
        return ss_logging.str();
    }
*/
/*



class ProtoRt_logger{
    ProtoRt_logger(){
        stream=stdout;
        buf=new char[101];
        buf[100]='\0';
        buf_size=100;
        buf_usesize=0;
    }
    ProtoRt_logger(char* file_name){
        strcat(file_name , ".txt");
        stream=freopen( file_name , "w" , stdout );
        buf=new char[101];
        buf[100]='\0';
        buf_size=100;
        buf_usesize=0;
    }
    ProtoRt_logger(FILE* file){
        stream=file;
        buf=new char[101];
        buf[100]='\0';
        buf_size=100;
        buf_usesize=0;
    }
    ~ProtoRt_logger(){
        this->flush();
        if(stream!=stdout){
            fclose(stream);
        }
        delete []buf;
    }
    void change_file(FILE* file){
        this->flush();
        stream=file;
    }
    void out_to_console(){
        this->flush();
        stream=stdout;
    }
    FILE* get_stream(){
        return stream;
    }
    //пишет полученное сообщение в буфер если есть в нём есть место , если нет то выводит буфер в поток
    bool write(char* message){
        int message_size=std::strlen(message);
        if(buf_size){

            if(message_size<=buf_size-buf_usesize){

                memmove(buf , message , message_size);
                buf_usesize+=message_size;

            }
            else{

                int first_part_size=buf_size-buf_usesize;
                memmove(buf+buf_usesize , message , first_part_size);
                buf_usesize+=first_part_size;

                if(!flush()){

                    //error;
                    delete[] message;
                    return false;

                }

                int second_part_size=message_size-first_part_size;
                memmove(buf+buf_usesize , message , second_part_size);
                buf_usesize+=second_part_size;
            }

        }
        else{
            if(fwrite(buf , sizeof(char) , buf_usesize , stream)!=message_size||!flush()){

                //error
                delete[] message;
                return false;
            }
            delete[] message;
            return true;
        }

        if(buf_usesize>buf_size*0,9){
            if(!flush()){
                //error
                delete[] message;
                return false;
            }
        }
        delete[] message;
        return true;
    }
    bool build_message(char* type_message , char* message){
        time_t ttime=time(NULL);
        tm* this_time=localtime(&ttime);
        std::stringstream ss_message;
        ss_message<<(int)(this_time->tm_mday)<<'/'<<(int)(this_time->tm_mon)<<'/'<<(int)(this_time->tm_year)
                 <<' '<<(int)(this_time->tm_hour)<<':'
                 <<(int)(this_time->tm_min)<<':'
                 <<(int)(this_time->tm_sec)<<' '<<type_message<<'-'<<message<<'\n';
        char* str=new char[ss_message.str().length()+1];
        strcpy(str , ss_message.str().c_str());
        str[ss_message.str().length()]='\0';
        return write(str);
    }
private:
    //выводит буфер в поток и возвращает смогли он это сделать
    bool flush(){
        if(fwrite(buf , sizeof(char) , buf_usesize , stream)!=buf_usesize){
            return false;
        }
        buf_usesize=0;
        return true;
    }
private:
    FILE *stream; // файл либо stdout
    char* buf;  // буффер для хранения данных перез записью
    int buf_size; //размер буффера
    int buf_usesize; //размер используемой части буффера
};



*/


class logger{
public:
    logger(){}
    virtual ~logger(){}
    virtual void log(const std::string &type , std::string &message){}
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

    virtual void log(const std::string &type ,std::string &message) override {
        message=build_message(type , message);
        file<<message<<std::endl;
    }

    virtual ~logger_file(){
        file.close();
    }

private:
    std::ofstream file;
};

class logger_stdout: public logger{
public:
    logger_stdout(){}
    virtual ~logger_stdout(){}
    virtual void log(const std::string &type ,std::string &message) override{
        message=build_message(type , message);
        std::cout<<message<<std::endl;
    }
};

class logger_component : public logger{
public:
    logger_component(boost::shared_ptr<logger> &log,  std::string &comp_name ,  std::string &comp_type){
        templ=comp_name+':'+comp_type;
        log_=log;
    }
    ~logger_component(){

    }
    virtual void log(const std::string &type ,  std::string &message) override{
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
    logger_node( boost::shared_ptr<logger> &log ,const std::string &node_name){
        templ=node_name;
        log_=log;
    }
    ~logger_node(){

    }
    virtual void log(const std::string &type ,  std::string &message) override{

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
