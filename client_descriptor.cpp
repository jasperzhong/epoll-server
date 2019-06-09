#include "client_descriptor.h"
#include <string>

ClientDescriptor::ClientDescriptor(int client_fd, in_addr client_addr, uint16_t client_port,
        uint32_t timeout) : client_fd_(client_fd), client_addr_(client_addr),
        client_port_(client_port), timeout_(timeout), state(State::UnAuthorized) {
    buffer_ = new uint8_t[10240];
}

ClientDescriptor::~ClientDescriptor() {
    delete []buffer_;
}

// 返回读到数据
int ClientDescriptor::readReady() {
    int bytes_read = 0;
    
    // 收满才退出
    while (1) {
        int n_read = recv(client_fd_, buffer_+bytes_read, 1024, 0);
        if (n_read <= 0)
            break;
        bytes_read += n_read;
    }
    return bytes_read;
}

int ClientDescriptor::writeReady(uint8_t*const begin, int bytes) {
    int bytes_write = 0;
    
    // 写满才退出
    while (bytes_write < bytes) {
        int n_write = send(client_fd_, begin + bytes_write, (bytes - bytes_write), 0);
        if (n_write < 0) {
            if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN) 
                continue;
            else if (errno == EPIPE)
                break;
            else
                return -1;
        }
        bytes_write += n_write;
    }
    return 0;
}
