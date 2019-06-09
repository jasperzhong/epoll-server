#ifndef SERVER_CONF_
#define SERVER_CONF_

#include <string>
#include <map>

class ServerConf {
public:
    void readConf();
public:
    std::map<std::string, std::string> conf;

    int port;
    int reconnect_interval;
    int retransfer_interval;

    std::string mysql_ip;
    int mysql_port;
    std::string db_name;
    std::string db_user;
    std::string db_pw;

    int rsp_timeout;
    int transfer_timeout;

    int main_log_size;
    int part_log_size;

    int print_screen;
    int tmp_packet;
    int tmp_socket;
    int dev_packet;
    int dev_socket;
};

#endif // !SERVER_CONF_