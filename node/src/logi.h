#ifndef LOGI_H
#define LOGI_H

#include <cstring>
#include <fstream>
#include <ctime>
#include <sstream>
#include <cstdio>
#include <string>

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
    //пишет полученное сообщение в буфер если есть в нём есть место , если нет то выводит в поток
    bool write(char* message){
        int message_size=std::strlen(message);
        if(buf_size){

            if(message_size<=buf_size-buf_usesize){

                memmove(buf , message , message_size);
                buf_usesize+=message_size;

            }
            else{

                int first_part_size=buf_size-buf_usesize;
                memmove(buf , message , first_part_size);
                buf_usesize+=first_part_size;

                if(!flush()){

                    //error;
                    return false;

                }

                int second_part_size=message_size-first_part_size;
                memmove(buf , message , second_part_size);
                buf_usesize+=second_part_size;
            }

        }
        else{
            if(fwrite(buf , sizeof(char) , buf_usesize , stream)!=message_size||!flush()){

                //error
                return false;
            }
            return true;
        }

        if(buf_usesize>buf_size*0,9){
            if(!flush()){
                //error
                return false;
            }
        }
        return true;
    }
protected:
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














/*
 * [time]:[node_name]/[component]:[type]:[message];
*/

#endif // LOGI_H
