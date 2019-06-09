#ifndef SERVER_TO_CLIENT_H_
#define SERVER_TO_CLIENT_H_

#include "packet_head.h"

/*
1. 构造的时候，把所有变量都初始化.
2. 对外统一接口，toString(int data_size=0)
3. 每个报文需要定义：
    (1) 数据长度
    (2) 每个变量
    (3) filling函数
    (4) 每个变量的get函数
 */
#include <iostream>
class ServerToClientBase {
public:
    ServerToClientBase(const PacketHead& packet_head) 
        : packet_head_(packet_head) {}
    
    ServerToClientBase(FunctionType function_type, uint16_t data_length, 
        uint16_t number=0) 
        : packet_head_(PacketType::kServer2Client, function_type, 
        data_length, number) {}

    virtual ~ServerToClientBase() {}

    // 所有子类这个函数都一样，只是分配空间大小不一样
    virtual bytePtr toString(int data_size=0) const {
        bytePtr packet_ptr(new uint8_t[data_size + 8]);
        packet_head_.filling(packet_ptr.get()); // 填充报头
        
        filling(packet_ptr.get()+8);

        return packet_ptr;
    }

    // 多态，子类调用自己的来填充除了报头以外的部分
    virtual void filling(uint8_t* const begin) const {}

    // 解析函数，只考虑包头外的部分，输入的指针从数据段开始
    virtual void parse(uint8_t* const begin) {};
protected:
    PacketHead packet_head_;
};


class ServerToClientAuthReq: public ServerToClientBase {
public:
    ServerToClientAuthReq(uint16_t reconnect_interval, uint16_t retansfer_interval);

    virtual bytePtr toString(int data_size=0) const;

    virtual void filling(uint8_t* const begin) const;

    virtual void parse(uint8_t* const begin);

    const int DATA_SIZE = 13 * 4; // bytes

// getter
public:
    uint16_t get_main_version_num_() const;
    uint8_t get_second1_version_num() const;
    uint8_t get_second2_version_num() const;
    uint16_t get_reconnect_interval() const;
    uint16_t get_retansfer_interval() const;
    uint8_t get_allow_null_end() const;
    auto get_auth_keys() const -> uint8_t(*)[32];
    uint32_t get_random_num() const;
    uint32_t get_svr_time() const;
private:

    // details
    uint16_t main_version_num_;
    uint8_t second1_version_num_;
    uint8_t second2_version_num_;
    uint16_t reconnect_interval_;
    uint16_t retansfer_interval_;
    uint8_t allow_null_end_;
    uint8_t auth_keys[32];
    uint32_t random_num_;
    uint32_t svr_time_;
};



class ServerToClientSysReq: public ServerToClientBase {
public:
    ServerToClientSysReq() 
        : ServerToClientBase(FunctionType::kSysReq, 0){
    }

    virtual bytePtr toString(int data_size=0) const;

    const int DATA_SIZE = 0; // bytes

// getter
public:

private:

    // details
};


class ServerToClientConfReq: public ServerToClientBase {
public:
    ServerToClientConfReq() 
        : ServerToClientBase(FunctionType::kConfReq, 0){
    }

    virtual bytePtr toString(int data_size=0) const;

    const int DATA_SIZE = 0; // bytes

// getter
public:
    
private:

    // details
};


class ServerToClientProcReq: public ServerToClientBase {
public:
    ServerToClientProcReq() 
        : ServerToClientBase(FunctionType::kProcReq, 0){
    }

    virtual bytePtr toString(int data_size=0) const;

    const int DATA_SIZE = 0; // bytes

// getter
public:
    
private:

    // details
};


class ServerToClientEthReq: public ServerToClientBase {
public:
    ServerToClientEthReq(uint16_t number) 
        : ServerToClientBase(FunctionType::kEthReq, 0,
        number){
    }

    virtual bytePtr toString(int data_size=0) const;

    const int DATA_SIZE = 0; // bytes

// getter
public:
    
private:

    // details
};


class ServerToClientUsbportReq: public ServerToClientBase {
public:
    ServerToClientUsbportReq() 
        : ServerToClientBase(FunctionType::kUsbportReq, 0){
    }

    virtual bytePtr toString(int data_size=0) const;

    const int DATA_SIZE = 0; // bytes

// getter
public:
    
private:

    // details
};


class ServerToClientUsbfileReq: public ServerToClientBase {
public:
    ServerToClientUsbfileReq() 
        : ServerToClientBase(FunctionType::kUsbfileReq, 0){
    }

    virtual bytePtr toString(int data_size=0) const;
    
    const int DATA_SIZE = 0; // bytes

// getter
public:
    
private:

    // details
};


class ServerToClientPrintportReq: public ServerToClientBase {
public:
    ServerToClientPrintportReq() 
        : ServerToClientBase(FunctionType::kPrintportReq, 0){
    }

    virtual bytePtr toString(int data_size=0) const;

    const int DATA_SIZE = 0; // bytes

// getter
public:
    
private:

    // details
};


class ServerToClientPrintqueueReq: public ServerToClientBase {
public:
    ServerToClientPrintqueueReq() 
        : ServerToClientBase(FunctionType::kPrintqueueReq, 0){
    }

    virtual bytePtr toString(int data_size=0) const;

    const int DATA_SIZE = 0; // bytes

// getter
public:
    
private:

    // details
};


class ServerToClientEndinfoReq: public ServerToClientBase {
public:
    ServerToClientEndinfoReq() 
        : ServerToClientBase(FunctionType::kEndinfoReq, 0){
    }

    virtual bytePtr toString(int data_size=0) const;

    const int DATA_SIZE = 0; // bytes

// getter
public:
    
private:

    // details
};


class ServerToClientDumbendReq: public ServerToClientBase {
public:
    ServerToClientDumbendReq(uint16_t number) 
        : ServerToClientBase(FunctionType::kDumbendReq, 0,
        number){
    }

    virtual bytePtr toString(int data_size=0) const;

    const int DATA_SIZE = 0; // bytes

// getter
public:
    
private:

    // details
};


class ServerToClientIpendReq: public ServerToClientBase {
public:
    ServerToClientIpendReq(uint16_t number) 
        : ServerToClientBase(FunctionType::kIpendReq, 0,
        number){
    }

    virtual bytePtr toString(int data_size=0) const;

    const int DATA_SIZE = 0; // bytes

// getter
public:
    
private:

    // details
};


class ServerToClientRcvAllAck: public ServerToClientBase {
public:
    ServerToClientRcvAllAck() 
        : ServerToClientBase(FunctionType::kRcvAllAck, 0){
    }

    virtual bytePtr toString(int data_size=0) const;

    const int DATA_SIZE = 0; // bytes

// getter
public:
    
private:

    // details
};



#endif // !SERVER_TO_CLIENT_H_