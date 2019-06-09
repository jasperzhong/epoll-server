#include "packet_head.h"

PacketHead::PacketHead(PacketType packet_type, FunctionType function_type, 
    uint16_t data_length, uint16_t number)
    : packet_type_(packet_type), function_type_(function_type),
    total_length_(data_length + 8), data_length_(data_length),
    number_(number) {
}


void PacketHead::filling(uint8_t*const begin) const {
    uint8_t net_0 = packet_type_;
    uint8_t net_1 = function_type_;    
    uint16_t net_2_3 = htons(total_length_);
    uint16_t net_4_5 = htons(number_); // pad
    uint16_t net_6_7 = htons(data_length_);

    memcpy(begin, reinterpret_cast<uint8_t*>(&net_0), 1); 
    memcpy(begin + 1, reinterpret_cast<uint8_t*>(&net_1), 1); 
    memcpy(begin + 2, reinterpret_cast<uint8_t*>(&net_2_3), 2);
    memcpy(begin + 4, reinterpret_cast<uint8_t*>(&net_4_5), 2); 
    memcpy(begin + 6, reinterpret_cast<uint8_t*>(&net_6_7), 2); 
}


void PacketHead::parse(uint8_t*const begin) {
    packet_type_ = *reinterpret_cast<uint8_t*>(begin);
    function_type_ = *reinterpret_cast<uint8_t*>(begin+1);
    total_length_ = ntohs(*reinterpret_cast<uint16_t*>(begin+2));
    number_ = ntohs(*reinterpret_cast<uint16_t*>(begin+4));
    data_length_ = ntohs(*reinterpret_cast<uint16_t*>(begin+6));
}

uint8_t PacketHead::get_packet_type() const {
    return packet_type_;
}

uint8_t PacketHead::get_function_type() const {
    return function_type_;
}

uint16_t PacketHead::get_total_length() const {
    return total_length_;
}

uint16_t PacketHead::get_number() const {
    return number_;
}

uint16_t PacketHead::get_data_length() const {
    return data_length_;
}
