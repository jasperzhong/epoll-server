#ifndef PACKET_HEAD_H_
#define PACKET_HEAD_H_

#include "common.h"
#include <string>
#include <cstring>
#include <arpa/inet.h>

class PacketHead {
public:
    PacketHead() {}
    PacketHead(PacketType packet_type, FunctionType function_type, 
        uint16_t data_length, uint16_t number=0);

    // 注意：必须在调用前已分配好内存 
    // 此函数只做填充报头
    void filling(uint8_t*const begin) const;

    // 解析一个报头到底是什么
    void parse(uint8_t*const begin);

// getter
public:
    uint8_t get_packet_type() const;
    uint8_t get_function_type() const;
    uint16_t get_total_length() const;
    uint16_t get_number() const;
    uint16_t get_data_length() const;

private:
    uint8_t packet_type_;
    uint8_t function_type_;
    uint16_t total_length_;
    uint16_t number_;
    uint16_t data_length_;
};

#endif // !PACKET_HEAD_H_