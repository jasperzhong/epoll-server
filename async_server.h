#ifndef ASYNC_SERVER_H_
#define ASYNC_SERVER_H_

#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <error.h>
#include <unistd.h>

#include <map>
#include <vector>
#include <string>
#include <thread>

#include "client_descriptor.h"
#include "logger.h"
#include "server_conf.h"

#include "/usr/include/mysql/mysql.h"

class AsyncServer {
public:
    AsyncServer(const ServerConf& conf);

    ~AsyncServer();

    void eventLoop(); // 死循环
private:
    bool setNonBlocking(int fd);


    bool handleAccept();

    bool handleClient(epoll_event ev);

private:
    const int MAX_CONN = 50000;
    const int ULIMIT = 4096;

    int open_file_cnt;
    int done_cnt;
    int listen_fd_, epoll_fd_;
    epoll_event *events_;
    std::map<int, ClientDescriptorPtr> clients_;
    ServerToClientBase* last_packet_ptr_;

    const ServerConf& conf_;
    Logger logger_;

    std::vector<std::string> sqls; 
    
    char sql_buffer[10240];

    MYSQL* mysql;
};

#endif // !ASYNC_SERVER_H_