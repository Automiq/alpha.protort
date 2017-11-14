#ifndef DATA_H
#define DATA_H

namespace alpha {
namespace protort {
namespace components {

/*!
 * \brief Функционал:
 * Распаковывание строки, представляющей информацию об объекте data в объект data.
 * Запаковывание объекта data в строку.
 */

struct data
{
    float val; //Значение, которое генерирует генератор
    std::time_t time; //Метка времени
    uint64_t address; // Адрес компонента

    /*!
     * \brief распаковывает строку string в data и
     * инициализирует информацией из него this
     */
    void unpack(std::string const & str){
        const data *d = reinterpret_cast<const data*> (str.data());
        val = d->val;
        time = d->time;
        address = d->address;
    }

    /*!
     * \brief запаковывает данные в строку
     */
    std::string pack(){
        std::string s(reinterpret_cast<char*>(this),sizeof(data));
        return s;
    }
};

} // namespace components
} // namespace protort
} // namespace alpha

#endif // DATA_H
