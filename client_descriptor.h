#ifndef CLIENT_DESCRIPTOR_H_
#define CLIENT_DESCRIPTOR_H_

#include <netinet/in.h>
#include <queue>
#include <memory>

#include "common/server_to_client.h"

constexpr int TOTAL_PACKETS = 10;

enum State {
    UnAuthorized,
    Authorized,
    Done
};

class ClientDescriptor {
public:
    ClientDescriptor(int client_fd, in_addr client_addr, uint16_t client_port,
        uint32_t timeout);

    ~ClientDescriptor();

    int readReady();
    
    int writeReady(uint8_t*const begin, int bytes);

public:
    State state; 
    
    uint8_t* buffer_; // 10K
    
    int client_fd_;
    in_addr client_addr_;
    uint16_t client_port_;
    uint32_t timeout_;


    // some vars...
    uint8_t async_port_num_; // 0/8/16
    uint32_t dev_id_;
    uint16_t mem_size_;


    std::queue<uint16_t> dumb_queue;
    std::queue<uint16_t> ip_queue;
    
};

typedef std::shared_ptr<ClientDescriptor> ClientDescriptorPtr;



#endif // !CLIENT_DESCRIPTOR_H_