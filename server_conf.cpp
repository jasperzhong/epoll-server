#include "server_conf.h"
#include <fstream>
#include <iostream>

void ServerConf::readConf() {
    std::ifstream fin("yzmond.conf");
    if (!fin.is_open()) {
        return;
    }

    std::string line;
    std::string lhs, rhs, str;
    while (getline(fin, line)) {
        lhs = rhs = "";
        for (auto ch: line) {
            if (ch == ' ' || ch == '\t') {
                continue;
            }
            if (ch == '/' || ch == '[') {
                break;
            }
            if (ch == '=') {
                lhs = str;
                str = "";
                continue;
            }
            str += ch;
        }
        rhs = str;
        str = "";
        if (lhs == "" || rhs == "")
            continue;
        // std::cout << lhs << "=" << rhs << std::endl;
        
        // 只认第一次
        if (conf[lhs] == "") {
            conf[lhs] = rhs;
        }
    }

    for(auto it = conf.begin(); it != conf.end(); ++it) {
        if (it->first == "监听端口号") {
            port = atoi(it->second.c_str());
        } else if (it->first == "设备连接间隔") {
            reconnect_interval = atoi(it->second.c_str());
        } else if (it->first == "设备采样间隔") {
            retransfer_interval == atoi(it->second.c_str());
        } else if (it->first == "数据库名") {
            db_name = it->second;
        } else if (it->first == "用户名") {
            db_user = it->second;
        } else if (it->first == "服务器IP地址") {
            mysql_ip = it->second;
        } else if (it->first == "服务器端口号") {
            mysql_port = atoi(it->second.c_str());
        } else if (it->first == "用户口令") {
            db_pw = it->second;
        } else if (it->first == "未应答超时") {
            rsp_timeout = atoi(it->second.c_str());
        } else if (it->first == "传输超时") {
            transfer_timeout = atoi(it->second.c_str());
        } else if (it->first == "分日志大小") {
            part_log_size = atoi(it->second.c_str());
        } else if (it->first == "主日志大小") {
            main_log_size = atoi(it->second.c_str());
        } else if (it->first == "屏幕显示") {
            print_screen = atoi(it->second.c_str());
        } else if (it->first == "tmp_packet") {
            tmp_packet = atoi(it->second.c_str());
        } else if (it->first == "tmp_socket") {
            tmp_socket = atoi(it->second.c_str());
        } else if (it->first == "dev_packet") {
            dev_packet = atoi(it->second.c_str());
        } else if (it->first == "dev_socket") {
            dev_socket = atoi(it->second.c_str());
        } else {
            throw std::runtime_error("wrong conf!");
        }  
    }
}