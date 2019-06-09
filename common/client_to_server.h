#ifndef CLIENT_TO_SERVER
#define CLIENT_TO_SERVER

#include "packet_head.h"
#include <vector>

/*
1. 构造的时候，把所有变量都初始化.
2. 对外统一接口，toString()
3. 每个报文需要定义：
    (1) 数据长度
    (2) 每个变量
    (3) filling函数
    (4) 每个变量的get函数
 */

class ClientToServerBase {
public:
    ClientToServerBase(const PacketHead& packet_head)
        : packet_head_(packet_head) {}

    ClientToServerBase(FunctionType function_type, uint16_t data_length,
        uint16_t number=0)
        : packet_head_(PacketType::kClient2Server, function_type,
        data_length, number) {}

    virtual ~ClientToServerBase() {}

    // 所有子类这个函数都一样，只是分配空间大小不一样
    virtual bytePtr toString(int data_size) const {
        bytePtr packet_ptr(new uint8_t[data_size + 8]);
        packet_head_.filling(packet_ptr.get()); // 填充报头

        filling(packet_ptr.get() + 8);

        return packet_ptr;
    }

    // 多态，子类调用自己的来填充除了报头以外的部分
    virtual void filling(uint8_t*const begin) const {};

    // 解析函数，只考虑包头外的部分，输入的指针从数据段开始
    virtual void parse(uint8_t*const begin) {};
public:
    PacketHead packet_head_;
};


class ClientToServerMinVersion: public ClientToServerBase {
public:
    ClientToServerMinVersion(const PacketHead& packet_head)
        : ClientToServerBase(packet_head){}

    ClientToServerMinVersion();

    virtual bytePtr toString() const;

    virtual void filling(uint8_t*const begin) const;

    virtual void parse(uint8_t*const begin);
public:
    const int DATA_SIZE = 1 * 4;

    // details
    uint16_t min_main_version_num_;
    uint8_t second1_version_num_;
    uint8_t second2_version_num_;
};


class ClientToServerAuthRsp: public ClientToServerBase {
public:
    ClientToServerAuthRsp(const PacketHead& packet_head)
        : ClientToServerBase(packet_head){}
    
    ClientToServerAuthRsp(); // TODO

    virtual bytePtr toString() const;

    virtual void filling(uint8_t*const begin) const;

    virtual void parse(uint8_t*const begin);

    void decrypt(uint8_t*const begin);
public:
    const int DATA_SIZE = 27 * 4;

    uint16_t cpu_mhz_;
    uint16_t mem_total_;
    uint16_t flash_size_;
    uint16_t internal_serial_num_;
    char group_serial_num_[16];
    char device_model_[16];
    char software_version_[16];
    uint8_t eth_num_;
    uint8_t sync_num_;
    uint8_t asyn_num_;
    uint8_t exch_num_;
    uint8_t usb_num_;
    uint8_t print_num_;
    // pad pad
    uint32_t dev_id_;
    uint8_t in_num_ = 1;
    // pad pad16
    uint8_t auth_keys[32];
    uint32_t random_num_;
};


class ClientToServerSysRsp: public ClientToServerBase {
public:
    ClientToServerSysRsp(const PacketHead& packet_head)
        : ClientToServerBase(packet_head){}
    
    ClientToServerSysRsp(); // TODO

    virtual bytePtr toString() const;

    virtual void filling(uint8_t*const begin) const;

    virtual void parse(uint8_t*const begin);
public:
    const int DATA_SIZE = 5 * 4;

    uint32_t user_cpu_time_;
    uint32_t nice_cpu_time_;
    uint32_t sys_cpu_time_;
    uint32_t idle_cpu_time_;
    uint32_t freed_mem_;
};


class ClientToServerConfRsp: public ClientToServerBase {
public:
    ClientToServerConfRsp(const PacketHead& packet_head)
        : ClientToServerBase(packet_head){}
    
    ClientToServerConfRsp(); // TODO

    virtual bytePtr toString() const {}; // TODO

    virtual void filling(uint8_t*const begin) const {}; // TODO

    virtual void parse(uint8_t*const begin);
public:
    std::string conf_; 
};


class ClientToServerProcRsp: public ClientToServerBase {
public:
    ClientToServerProcRsp(const PacketHead& packet_head)
        : ClientToServerBase(packet_head){}
    
    ClientToServerProcRsp(); // TODO

    virtual bytePtr toString() const {}; // TODO

    virtual void filling(uint8_t*const begin) const {}; // TODO

    virtual void parse(uint8_t*const begin);
public:
    std::string info_; 
};


class ClientToServerEthRsp: public ClientToServerBase {
public:
    ClientToServerEthRsp(const PacketHead& packet_head)
        : ClientToServerBase(packet_head){}
    
    ClientToServerEthRsp(); // TODO

    virtual bytePtr toString() const;

    virtual void filling(uint8_t*const begin) const;

    virtual void parse(uint8_t*const begin);
public:
    const int DATA_SIZE = 31 * 4;

    uint8_t is_exist_;
    uint8_t is_config_;
    uint8_t up_down_;
    // pad
    uint8_t mac_0_;
    uint8_t mac_1_;
    uint8_t mac_2_;
    uint8_t mac_3_;
    uint8_t mac_4_;
    uint8_t mac_5_;
    uint16_t opt_;
    
    uint32_t ip_;
    uint32_t mask_;
    uint32_t ip_1_;
    uint32_t mask_1_;
    uint32_t ip_2_;
    uint32_t mask_2_;
    uint32_t ip_3_;
    uint32_t mask_3_;
    uint32_t ip_4_;
    uint32_t mask_4_;
    uint32_t ip_5_;
    uint32_t mask_5_;

    uint32_t rcv_bytes_;
    uint32_t rcv_packets_;
    uint32_t rcv_err_packets_;
    uint32_t rcv_drop_packets_;
    uint32_t rcv_fifo_packets_;
    uint32_t rcv_frames_;
    uint32_t rcv_compress_packets_;
    uint32_t rcv_broadcast_packets_;

    uint32_t snd_bytes_;
    uint32_t snd_packets_;
    uint32_t snd_err_packets_;
    uint32_t snd_drop_packets_;
    uint32_t snd_fifo_packets_;
    uint32_t snd_frames_;
    uint32_t snd_compress_packets_;
    uint32_t snd_broadcast_packets_;
};

class ClientToServerUsbportRsp: public ClientToServerBase {
public:
    ClientToServerUsbportRsp(const PacketHead& packet_head)
        : ClientToServerBase(packet_head){}

    ClientToServerUsbportRsp();

    virtual bytePtr toString() const;

    virtual void filling(uint8_t*const begin) const;

    virtual void parse(uint8_t*const begin);
public:
    const int DATA_SIZE = 1 * 4;

    // details
    uint8_t is_plug_usb_;
};


class ClientToServerUsbfileRsp: public ClientToServerBase {
public:
    ClientToServerUsbfileRsp(const PacketHead& packet_head)
        : ClientToServerBase(packet_head){}
    
    ClientToServerUsbfileRsp(); // TODO

    virtual bytePtr toString() const {}; // TODO

    virtual void filling(uint8_t*const begin) const {}; // TODO

    virtual void parse(uint8_t*const begin);
public:
    std::string info_; 
};


class ClientToServerPrintportRsp: public ClientToServerBase {
public:
    ClientToServerPrintportRsp(const PacketHead& packet_head)
        : ClientToServerBase(packet_head){}
    
    ClientToServerPrintportRsp(); // TODO

    virtual bytePtr toString() const; // TODO

    virtual void filling(uint8_t*const begin) const; // TODO

    virtual void parse(uint8_t*const begin);
public:
    const int DATA_SIZE = 9 * 4;

    uint8_t is_on_;
    // pad
    uint16_t tasks_;
    char printer_name_[32];
};


class ClientToServerPrintqueueRsp: public ClientToServerBase {
public:
    ClientToServerPrintqueueRsp(const PacketHead& packet_head)
        : ClientToServerBase(packet_head){}
    
    ClientToServerPrintqueueRsp(); // TODO

    virtual bytePtr toString() const {}; // TODO

    virtual void filling(uint8_t*const begin) const {}; // TODO

    virtual void parse(uint8_t*const begin);
public:
    std::string info_; 
};

class ClientToServerEndinfoRsp: public ClientToServerBase {
public:
    ClientToServerEndinfoRsp(const PacketHead& packet_head)
        : ClientToServerBase(packet_head){}
    
    ClientToServerEndinfoRsp(); // TODO

    virtual bytePtr toString() const; // TODO

    virtual void filling(uint8_t*const begin) const; // TODO

    virtual void parse(uint8_t*const begin);
public:
    const int DATA_SIZE =  16 + 254 + 2;

    uint8_t dump_end_[16];
    uint8_t ip_end_[254];
    uint16_t end_num_; 
};

// 24 * 4
struct VirtualScreen {
    uint8_t no;
    // pad
    uint16_t tcp_port;
    uint32_t ip;
    char proto[12];
    char state[8];
    char hint[24];
    char type[12];
    uint32_t time;
    uint32_t snd_end_bytes;
    uint32_t rcv_end_bytes;
    uint32_t snd_remote_bytes;
    uint32_t rcv_remote_bytes;
    uint32_t min_ping;
    uint32_t avg_ping;
    uint32_t max_ping;
};

class ClientToServerDumbendRsp: public ClientToServerBase {
public:
    ClientToServerDumbendRsp(const PacketHead& packet_head)
        : ClientToServerBase(packet_head){}
    
    ClientToServerDumbendRsp(); // TODO

    virtual bytePtr toString() const {}; // TODO

    virtual void filling(uint8_t*const begin) const {}; // TODO

    virtual void parse(uint8_t*const begin);

    void parseVS(uint8_t*const begin);
public:
    uint8_t port; // 1-254
    uint8_t config_port; // 1-254
    uint8_t active_vs;
    uint8_t total_vs;
    uint32_t ip;
    char type[12];
    char state[8];
    std::vector<VirtualScreen> vs;
};

class ClientToServerIpendRsp: public ClientToServerBase {
public:
    ClientToServerIpendRsp(const PacketHead& packet_head)
        : ClientToServerBase(packet_head){}
    
    ClientToServerIpendRsp(); // TODO

    virtual bytePtr toString() const {}; // TODO

    virtual void filling(uint8_t*const begin) const {}; // TODO

    virtual void parse(uint8_t*const begin);

    void parseVS(uint8_t*const begin);
public:
    uint8_t port; // 1-254
    uint8_t config_port; // 1-254
    uint8_t active_vs;
    uint8_t total_vs;
    uint32_t ip;
    char type[12];
    char state[8];
    std::vector<VirtualScreen> vs; 
};


#endif // !CLIENT_TO_SERVER