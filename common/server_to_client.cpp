#include "server_to_client.h"


/* ServerToClientAuthReq */

ServerToClientAuthReq::ServerToClientAuthReq(uint16_t reconnect_interval, uint16_t retansfer_interval) 
        : ServerToClientBase(FunctionType::kAuthReq, 52) {
    // do {
    //     main_version_num_ = rand_uint16();
    // } while(main_version_num_ == 0x0000 || main_version_num_ == 0xFFFF);

    // second1_version_num_ = rand_uint8();
    // second2_version_num_ = rand_uint8();
    main_version_num_ = 2;
    second1_version_num_ = 1;
    second2_version_num_ = 3;
    reconnect_interval_ = reconnect_interval;
    retansfer_interval_ = retansfer_interval;
    allow_null_end_ = rand_bool();

    random_num_ = rand_uint32();
    svr_time_ = rand_svr_time();
    svr_time_ ^= 0xFFFFFFFF;
    auto pos = random_num_ % 4093;

    for (int i = 0; i != 32; ++i) {
        auth_keys[i] = AUTH_KEYS[i] ^ SECRET[pos++%4093];
    }    
}

bytePtr ServerToClientAuthReq::toString(int data_size) const {
    return ServerToClientBase::toString(DATA_SIZE);
}

void ServerToClientAuthReq::filling(uint8_t* const begin) const {
    uint16_t net_0_1 = htons(main_version_num_);
    uint8_t net_2 = second1_version_num_;
    uint8_t net_3 = second2_version_num_;
    uint16_t net_4_5 = htons(reconnect_interval_);
    uint16_t net_6_7 = htons(retansfer_interval_);
    uint8_t net_8 = allow_null_end_;
    uint8_t net_9 = 0;
    uint16_t net_10_11 = 0;
    // net_12_43 is auth_keys[32]
    uint32_t net_44_47 = htonl(random_num_);
    uint32_t net_48_51 = htonl(svr_time_);

    memcpy(begin, reinterpret_cast<uint8_t*>(&net_0_1), 2);
    memcpy(begin+2, reinterpret_cast<uint8_t*>(&net_2), 1);
    memcpy(begin+3, reinterpret_cast<uint8_t*>(&net_3), 1);
    memcpy(begin+4, reinterpret_cast<uint8_t*>(&net_4_5), 2);
    memcpy(begin+6, reinterpret_cast<uint8_t*>(&net_6_7), 2);
    memcpy(begin+8, reinterpret_cast<uint8_t*>(&net_8), 1);
    memcpy(begin+9, reinterpret_cast<uint8_t*>(&net_9), 1);
    memcpy(begin+10, reinterpret_cast<uint8_t*>(&net_10_11), 2);
    memcpy(begin+12, auth_keys, 32);
    memcpy(begin+44, reinterpret_cast<uint8_t*>(&net_44_47), 4);
    memcpy(begin+48, reinterpret_cast<uint8_t*>(&net_48_51), 4);
}


void ServerToClientAuthReq::parse(uint8_t* const begin) {
    main_version_num_ = ntohs(*reinterpret_cast<uint16_t*>(begin));
    second1_version_num_ = *reinterpret_cast<uint8_t*>(begin+2);
    second2_version_num_ = *reinterpret_cast<uint8_t*>(begin+3);
    reconnect_interval_ = ntohs(*reinterpret_cast<uint16_t*>(begin+4));
    retansfer_interval_ = ntohs(*reinterpret_cast<uint16_t*>(begin+6));
    allow_null_end_ = *reinterpret_cast<uint8_t*>(begin+8);
    memcpy(auth_keys, begin+12, 32);
    random_num_ = ntohl(*reinterpret_cast<uint32_t*>(begin+44));
    svr_time_ = ntohl(*reinterpret_cast<uint32_t*>(begin+48));
}

uint16_t ServerToClientAuthReq::get_main_version_num_() const {
    return main_version_num_;
}
uint8_t ServerToClientAuthReq::get_second1_version_num() const {
    return second1_version_num_;
}
uint8_t ServerToClientAuthReq::get_second2_version_num() const {
    return second2_version_num_;
}
uint16_t ServerToClientAuthReq::get_reconnect_interval() const {
    return reconnect_interval_;
}
uint16_t ServerToClientAuthReq::get_retansfer_interval() const {
    return retansfer_interval_;
}
uint8_t ServerToClientAuthReq::get_allow_null_end() const {
    return allow_null_end_;
}
auto ServerToClientAuthReq::get_auth_keys() const -> uint8_t(*)[32] {
    return const_cast< uint8_t(*)[32]>(&auth_keys);
}
uint32_t ServerToClientAuthReq::get_random_num() const {
    return random_num_;
}
uint32_t ServerToClientAuthReq::get_svr_time() const {
    return svr_time_;
}




bytePtr ServerToClientSysReq::toString(int data_size) const {
    return ServerToClientBase::toString(DATA_SIZE);
}

bytePtr ServerToClientConfReq::toString(int data_size) const {
    return ServerToClientBase::toString(DATA_SIZE);
}

bytePtr ServerToClientProcReq::toString(int data_size) const {
    return ServerToClientBase::toString(DATA_SIZE);
}

bytePtr ServerToClientEthReq::toString(int data_size) const {
    return ServerToClientBase::toString(DATA_SIZE);
}

bytePtr ServerToClientUsbportReq::toString(int data_size) const {
    return ServerToClientBase::toString(DATA_SIZE);
}

bytePtr ServerToClientUsbfileReq::toString(int data_size) const {
    return ServerToClientBase::toString(DATA_SIZE);
}

bytePtr ServerToClientPrintportReq::toString(int data_size) const {
    return ServerToClientBase::toString(DATA_SIZE);
}

bytePtr ServerToClientPrintqueueReq::toString(int data_size) const {
    return ServerToClientBase::toString(DATA_SIZE);
}

bytePtr ServerToClientEndinfoReq::toString(int data_size) const {
    return ServerToClientBase::toString(DATA_SIZE);
}

bytePtr ServerToClientDumbendReq::toString(int data_size) const {
    return ServerToClientBase::toString(DATA_SIZE);
}

bytePtr ServerToClientIpendReq::toString(int data_size) const {
    return ServerToClientBase::toString(DATA_SIZE);
}

bytePtr ServerToClientRcvAllAck::toString(int data_size) const {
    return ServerToClientBase::toString(DATA_SIZE);
}

