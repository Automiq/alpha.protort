#ifndef DATA_H
#define DATA_H

namespace alpha {
namespace protort {
namespace components {

struct data
{
    float val; //значение, которое генерирует генератор
    std::time_t time; //метка времени

    //!\brief распаковывает строку и инициализирует this
    void unpack(std::string const & str){
        const data *d = reinterpret_cast<const data*> (str.data());
        val = d->val;
        time = d->time;
    }

    //!\brief запаковывает данные в строку
    std::string pack(){
        std::string s(reinterpret_cast<char*>(this),sizeof(data));
        return s;
    }
};

} // namespace components
} // namespace protort
} // namespace alpha

#endif // DATA_H
