#ifndef PACKET_HEADER_H
#define PACKET_HEADER_H

namespace alpha {
namespace protort {
namespace link {

/*!
 * \brief Заголовок пакета уровня библиотеки link
 *
 * Каждый пакет, передаваемый по каналу связи через библиотеку
 * alpha.protort.link предваряется данным заголовком. Заголовок всегда имеет
 * фиксированный размер и реализует технику length-prefix framing, то есть
 * заголовок всегда содержит размер пакета в байтах. Таким образом прочитав
 * заголовок, всегда точно понятно сколько байтов пакета затем следует прочитать
 * и где заканчивается один пакет и начинается следующий.
 */
struct packet_header
{
    /*!
     * \brief Размер пакета
     */
    uint32_t packet_size;
};

//! Размер заголовка пакета в байтах
static const uint32_t header_size = sizeof(packet_header);

//! Максимально допустимый размер пакета
static const uint32_t max_packet_size = 1024*1024;

} //namespace link
} //namespace protort
} //namespace alpha

#endif // PACKET_HEADER_H
