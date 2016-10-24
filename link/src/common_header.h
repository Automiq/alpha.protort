#ifndef COMMON_HEADER_H
#define COMMON_HEADER_H

namespace alpha {
namespace protort {
namespace link {

static const uint32_t max_packet_size = 1024*1024;

struct packet_header
{
    uint32_t packet_size;
};

static const uint32_t header_size = sizeof(packet_header);

} //namespace link
} //namespace protort
} //namespace alpha

#endif
