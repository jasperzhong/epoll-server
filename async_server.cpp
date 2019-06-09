#include "async_server.h"
#include "common/server_to_client.h"
#include "common/client_to_server.h"

void insert_batch(MYSQL* mysql, std::vector<std::string>& sqls) {
    mysql_query(mysql, "START TRANSACTION");

    for (auto& sql: sqls) {
        if(mysql_query(mysql, sql.c_str())) {
            // std::cout << "mysql_query failed(), error reason:" << mysql_error(mysql) << std::endl;
        }
    }
    sqls.clear();

    mysql_query(mysql, "COMMIT");
}

AsyncServer::AsyncServer(const ServerConf& conf)
    : listen_fd_(-1), epoll_fd_(-1), open_file_cnt(0), done_cnt(0), conf_(conf) {
    
    sockaddr_in sin = {0};
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_family = AF_INET;
    sin.sin_port = htons(conf.port);

    listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd_ <= 0)
        throw std::runtime_error("socket() failed, error code:" + std::to_string(errno));
    
    int opt = 1;
    if (setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR , &opt, sizeof(opt)) < 0) 
		throw std::runtime_error("setsockopt() failed, error code:" + std::to_string(errno));

    if (bind(listen_fd_, reinterpret_cast<sockaddr*>(&sin), sizeof(sin)) == -1) 
        throw std::runtime_error("bind() failed, error code:" + std::to_string(errno));
    
    if (setNonBlocking(listen_fd_) == false) 
        throw std::runtime_error("setNonBlocking() failed, error code:" + std::to_string(errno));
    
    if (listen(listen_fd_, MAX_CONN) == -1)
        throw std::runtime_error("listen() failed, error code:" + std::to_string(errno));
    
    epoll_fd_ = epoll_create(MAX_CONN);
    if (epoll_fd_ == -1)
        throw std::runtime_error("epoll_create() failed, errno code:" + std::to_string(errno));

    epoll_event e_event;
    e_event.events = EPOLLIN;
    e_event.data.fd = listen_fd_;

    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, listen_fd_, &e_event) == -1)
        throw std::runtime_error("epoll_ctl() failed, error code:" + std::to_string(errno));

    events_ = new epoll_event[MAX_CONN];

    if (conf.main_log_size > 0)
        logger_.start();
    

    if ((mysql = mysql_init(NULL)) == NULL) {
        throw std::runtime_error("mysql_init() failed, error reason:" + std::string(mysql_error(mysql)));
    }

    if (mysql_real_connect(mysql,"localhost",
         conf.db_user.c_str(), conf.db_pw.c_str(), conf.db_name.c_str(), 0, NULL, 0)==NULL) {
        throw std::runtime_error("mysql_real_connect() failed, error reason:" + std::string(mysql_error(mysql)));
    }

    mysql_set_character_set(mysql, "gbk"); 

    logger_.info("已连接数据库");
    logger_.info("服务端已启动, 监听端口:" + std::to_string(conf.port));
    logger_.flush();
}

AsyncServer::~AsyncServer() {
    if (listen_fd_ != -1)
        close(listen_fd_);
    if (epoll_fd_ != -1) 
        close(epoll_fd_);
    delete []events_;
}

void AsyncServer::eventLoop() {
    while (1) {
        int ndfs = epoll_wait(epoll_fd_, events_, MAX_CONN, 1000);
        if (ndfs > 0) {
            for (int i = 0;i < ndfs; ++i) {
                if (events_[i].data.fd == listen_fd_) {
                    // 防止errno 24
                    if (open_file_cnt < ULIMIT){
                        handleAccept();
                        open_file_cnt++;
                    }
                } else {
                    handleClient(events_[i]);
                }
            }
        }
        logger_.flush();
        
        insert_batch(mysql, sqls);
    }
}

bool AsyncServer::setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        return false;
    flags |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flags) == -1)
        return false;
    return true;
}


bool AsyncServer::handleAccept() {
    sockaddr_in client_sin;
    socklen_t sin_size = sizeof(client_sin);
    

    int client_fd = accept(listen_fd_, reinterpret_cast<sockaddr *>(&client_sin), &sin_size);
    if (client_fd == -1) {
        logger_.warning("accept() failed, error code: " + std::to_string(errno));
        return false;
    }

    if (setNonBlocking(client_fd) == false) {
        logger_.warning("setNonBlocking() failed, error code: " + std::to_string(errno));
        return false;
    }

    ClientDescriptorPtr client(new ClientDescriptor(
        client_fd, client_sin.sin_addr, client_sin.sin_port, 0
    )); 

    epoll_event ev;
    ev.data.fd = client_fd;
    ev.events = EPOLLIN | EPOLLET | EPOLLOUT;

    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, client_fd, &ev) == -1) {
        logger_.warning("epoll_ctl() failed, error code: " + std::to_string(errno));
        return false;
    }
    
    clients_[client_fd] = client;

    logger_.info("建立未认证连接" + std::string(inet_ntoa(client_sin.sin_addr)) + ":" + std::to_string(ntohs(client_sin.sin_port))); 

    // 1. 发认证包
    do {
        ServerToClientAuthReq packet(conf_.reconnect_interval, conf_.retransfer_interval);
        auto data = packet.toString();
        if (client->writeReady(data.get(), packet.DATA_SIZE + 8) < 0) {
            logger_.fatal("write failed, errno code:" + std::to_string(errno));
            break;
        }
        logger_.info("向未认证连接(IP=" + std::string(inet_ntoa(client_sin.sin_addr))  + ")发送认证");
        logger_.info("向未认证连接(IP=" + std::string(inet_ntoa(client_sin.sin_addr))  + ")发送60字节");
        logger_.info("(发送数据为:)");
        logger_.print_data(data.get(), packet.DATA_SIZE+8);
    } while(0);

    return true;
}

// 状态机
bool AsyncServer::handleClient(epoll_event ev) {
    auto client = clients_[ev.data.fd];

    if (ev.events & EPOLLIN) {
        int read_bytes = client->readReady();
        if (read_bytes == 0) {
            logger_.fatal("收到数据大小为0字节! open_file_cnt:" + std::to_string(open_file_cnt) + " done_cnt:" + std::to_string(done_cnt));
            close(client->client_fd_);
            open_file_cnt--;
            return false;
        }
        PacketHead ph;
        ph.parse(client->buffer_);

        logger_.info("收到数据" + std::to_string(read_bytes) +"字节");
        logger_.info("(收到数据为:)");

        logger_.print_data(client->buffer_, read_bytes);
        
        switch (ph.get_function_type())
        {
        case FunctionType::kMinVersion: {
                ClientToServerMinVersion packet(ph);
                packet.parse(client->buffer_+8);
                logger_.fatal("您的版本低于客户端最低要求版本:" + 
                    std::to_string(packet.min_main_version_num_) + "." + 
                    std::to_string(packet.second1_version_num_) + "." + 
                    std::to_string(packet.second2_version_num_) + ". 关闭TCP");
                
                close(client->client_fd_);
                open_file_cnt--;
                break;
            }
        case FunctionType::kAuthRsp: {
            ClientToServerAuthRsp packet(ph);
            packet.parse(client->buffer_+8);

            // decrypt
            uint32_t pos = packet.random_num_ % 4093;
            for (int i = 8;i < 8+104; ++i)
                client->buffer_[i] ^= SECRET[pos++%4093];
            packet.decrypt(client->buffer_+8);
            int flag = true;
            for (int i = 0;i < 32; ++i) {
                if (char(packet.auth_keys[i]) != AUTH_KEYS[i]) {
                    flag = false;
                    break;
                }
            }

            client->async_port_num_ = packet.asyn_num_;
            client->mem_size_ = packet.mem_total_;


            if (!flag) {
                logger_.fatal("认证失败!(IP=" + std::string(inet_ntoa(client->client_addr_)) + ". 关闭TCP连接.");
                close(client->client_fd_);
                clients_.erase(client->client_fd_);
                open_file_cnt--;
            } else {
                logger_.info("网点加密机认证成功!(IP=" + std::string(inet_ntoa(client->client_addr_)) + ").");
                logger_.info("(收到数据为:)");
                logger_.info(" dev_id:" + std::to_string(packet.dev_id_) + 
                    " 设备型号:" + std::string(packet.device_model_) + 
                    " 序列号:" + std::string(packet.group_serial_num_) + 
                    " 软件版本:" + std::string(packet.software_version_) + 
                    " CPU主频:" + std::to_string(packet.cpu_mhz_) + "Mhz" + 
                    " 内存:" + std::to_string(packet.mem_total_) + "MB" + 
                    " FLASH:" + std::to_string(packet.flash_size_) + "MB" + 
                    " 以太口数量:" + std::to_string(packet.eth_num_) + 
                    " 同步口数量:" + std::to_string(packet.sync_num_) + 
                    " 交换机数量:" + std::to_string(packet.exch_num_) + 
                    " USB口数量:" + (packet.usb_num_==0?std::string("不存在"):std::string("存在")) + 
                    " 打印口数量:" + (packet.print_num_==0?std::string("不存在"):std::string("存在")));
                client->state = State::Authorized;
            }
    
            client->dev_id_ = packet.dev_id_;
            char cur_time[80];
            time_t t = time(0);
            struct tm *p = gmtime(&t);
            strftime(cur_time, 80, "%Y-%m-%d %H:%M:%S", p);

            sprintf(sql_buffer, "insert into devstate_base(devstate_base_devid, devstate_base_devno, devstate_base_time, \
                 devstate_base_ipaddr, devstate_base_sid, devstate_base_type, devstate_base_version, \
                 devstate_base_cpu, devstate_base_sdram, devstate_base_flash, devstate_base_ethnum, devstate_base_syncnum, \
                 devstate_base_asyncnum, devstate_base_switchnum, devstate_base_usbnum, devstate_base_prnnum) \
                 values('%u', '%d', '%s', '%s', '%s-%d', '%s', '%s', %d, %d, %d,  %d, %d, %d, %d,  '%s','%s')", 
                packet.dev_id_, 1, cur_time, inet_ntoa(client->client_addr_), packet.group_serial_num_, packet.internal_serial_num_, 
                packet.software_version_, packet.device_model_,  packet.cpu_mhz_, packet.mem_total_, packet.flash_size_, 
                packet.eth_num_, packet.sync_num_, packet.asyn_num_, packet.exch_num_, 
                packet.usb_num_==0?"不存在":"存在",  packet.print_num_==0?"不存在":"存在"
            );
            sqls.push_back(sql_buffer);
        
            // 2. 取系统信息
            do {
                ServerToClientSysReq packet;
                auto data = packet.toString();
                last_packet_ptr_ = &packet;
                if (client->writeReady(data.get(), packet.DATA_SIZE + 8) < 0) {
                    logger_.fatal("write failed, errno code:" + std::to_string(errno));
                    break;
                }
                logger_.info("向已认证连接(IP=" + std::string(inet_ntoa(client->client_addr_))  + ")请求系统信息");
                logger_.info("(发送数据为:)");
                logger_.print_data(data.get(), packet.DATA_SIZE+8);
            } while(0);
    
            break;
        }
        case FunctionType::kSysRsp: {
            ClientToServerSysRsp packet(ph);
            packet.parse(client->buffer_+8);


            logger_.info("收到系统信息!(IP=" + std::string(inet_ntoa(client->client_addr_)) + ").");
            logger_.info("(收到数据为:)");
            logger_.info("user CPU time:" + std::to_string(packet.user_cpu_time_) + 
                    " nice CPU time:" + std::to_string(packet.nice_cpu_time_) + 
                    " system CPU time:" + std::to_string(packet.sys_cpu_time_) + 
                    " idle CPU time:" + std::to_string(packet.idle_cpu_time_) + 
                    " freed memory:" + std::to_string(packet.freed_mem_));   

            float cpu_used = float(packet.user_cpu_time_ + packet.sys_cpu_time_) / (packet.user_cpu_time_ + 
            packet.nice_cpu_time_ + packet.sys_cpu_time_ + packet.idle_cpu_time_);
            float sdram_used = float(packet.freed_mem_) / (client->mem_size_);
            // sprintf(sql_buffer, "update devstate_base(devstate_base_cpu_uesd, devstate_base_sdram_used)" 
            //     " values(%.2f, %.2f)", (cpu_used, sdram_used));
            // sqls.push_back(sql_buffer);

            // 3. 取配置信息

            do {
                ServerToClientConfReq packet;
                auto data = packet.toString();
                last_packet_ptr_ = &packet;
                if (client->writeReady(data.get(), packet.DATA_SIZE + 8) < 0) {
                    logger_.fatal("write failed, errno code:" + std::to_string(errno));
                    break;
                }
                logger_.info("向已认证连接(IP=" + std::string(inet_ntoa(client->client_addr_))  + ")请求配置信息");
                logger_.info("(发送数据为:)");
                logger_.print_data(data.get(), packet.DATA_SIZE+8);
            } while(0);

            break;
        }
        case FunctionType::kConfRsp: {
            ClientToServerConfRsp packet(ph);
            packet.parse(client->buffer_+8);


            logger_.info("收到配置信息!(IP=" + std::string(inet_ntoa(client->client_addr_)) + ").");
            logger_.info("(收到数据为:)");
            logger_.info(packet.conf_);   

            // sprintf(sql_buffer, "insert into devstate_base(devstate_base_config) values('%s')", packet.conf_.c_str());
            // sqls.push_back(sql_buffer);

            // 4. 取进程信息

            do {
                ServerToClientProcReq packet;
                auto data = packet.toString();
                last_packet_ptr_ = &packet;
                if (client->writeReady(data.get(), packet.DATA_SIZE + 8) < 0) {
                    logger_.fatal("write failed, errno code:" + std::to_string(errno));
                    break;
                }
                logger_.info("向已认证连接(IP=" + std::string(inet_ntoa(client->client_addr_))  + ")请求进程信息");
                logger_.info("(发送数据为:)");
                logger_.print_data(data.get(), packet.DATA_SIZE+8);
            } while(0);

            break;
        };
        case FunctionType::kProcRsp: {
            ClientToServerProcRsp packet(ph);
            packet.parse(client->buffer_+8);


            logger_.info("收到进程信息!(IP=" + std::string(inet_ntoa(client->client_addr_)) + ").");
            logger_.info("(收到数据为:)");
            logger_.info(packet.info_);   

            // sprintf(sql_buffer, "insert into devstate_base(devstate_base_process) values('%s')", packet.info_.c_str());
            // sqls.push_back(sql_buffer);

            // 5. 取以太口信息

            do {
                ServerToClientEthReq packet(0x0000);
                auto data = packet.toString();
                last_packet_ptr_ = &packet;
                if (client->writeReady(data.get(), packet.DATA_SIZE + 8) < 0) {
                    logger_.fatal("write failed, errno code:" + std::to_string(errno));
                    break;
                }
                logger_.info("向已认证连接(IP=" + std::string(inet_ntoa(client->client_addr_))  + ")请求以太口0信息");
                logger_.info("(发送数据为:)");
                logger_.print_data(data.get(), packet.DATA_SIZE+8);
            } while(0);
            break;
        }
        case FunctionType::kEthRsp: {
            ClientToServerEthRsp packet(ph);
            packet.parse(client->buffer_+8);

            uint16_t number = packet.packet_head_.get_number();
            if (number == 0x0000) {
                do {
                    ServerToClientEthReq packet(0x0001);
                    auto data = packet.toString();
                    last_packet_ptr_ = &packet;
                    if (client->writeReady(data.get(), packet.DATA_SIZE + 8) < 0) {
                        logger_.fatal("write failed, errno code:" + std::to_string(errno));
                        break;
                    }
                    logger_.info("向已认证连接(IP=" + std::string(inet_ntoa(client->client_addr_))  + ")请求以太口1信息");
                    logger_.info("(发送数据为:)");
                    logger_.print_data(data.get(), packet.DATA_SIZE+8);
                } while(0);             
            } else {
                do {
                    ServerToClientUsbportReq packet;
                    auto data = packet.toString();
                    last_packet_ptr_ = &packet;
                    if (client->writeReady(data.get(), packet.DATA_SIZE + 8) < 0) {
                        logger_.fatal("write failed, errno code:" + std::to_string(errno));
                        break;
                    }
                    logger_.info("向已认证连接(IP=" + std::string(inet_ntoa(client->client_addr_))  + ")请求USB口是否存在U盘信息");
                    logger_.info("(发送数据为:)");
                    logger_.print_data(data.get(), packet.DATA_SIZE+8);
                } while(0);   
            }

            logger_.info("收到以太口" + std::to_string(number) + "信息!(IP=" + std::string(inet_ntoa(client->client_addr_)) + ").");
            in_addr inaddr;
            inaddr.s_addr = packet.ip_; 
            logger_.info("E" + std::to_string(number) + ":" + std::string(inet_ntoa(inaddr)) + "/" + 
                std::to_string(packet.mask_) + "\t" + 
                std::to_string(packet.mac_0_) + std::to_string(packet.mac_1_) + ":" + 
                std::to_string(packet.mac_2_) + std::to_string(packet.mac_3_) + ":" + 
                std::to_string(packet.mac_4_) + std::to_string(packet.mac_5_) + " " + 
                (packet.up_down_?std::string("Up"):std::string("Down")) + " " + 
                std::to_string(packet.opt_) + " \n" + 
                "Tx:" + std::to_string(packet.snd_bytes_) + " bytes/" + std::to_string(packet.snd_packets_) + " pks\t" + 
                "Rx:" + std::to_string(packet.rcv_bytes_) + " bytes/" + std::to_string(packet.rcv_packets_) + " pks"
            );

            // sprintf("insert into devstate_base(devstate_base_eth0_ip, devstate_base_eth0_mask) values('%s', '%d')", 
            //     inet_ntoa(inaddr), packet.mask_); // ...
            // sqls.push_back(sql_buffer);

            break;
        }
        case FunctionType::kUsbportReq: {
            ClientToServerUsbportRsp packet(ph);
            packet.parse(client->buffer_+8);

            logger_.info("收到USB口信息!(IP=" + std::string(inet_ntoa(client->client_addr_)) + ").");
            logger_.info("(收到数据为:)");
            logger_.info(packet.is_plug_usb_==0?std::string("是"):std::string("否"));   

            // 需要请求U盘文件列表信息
            if (packet.is_plug_usb_) {
                do {
                    ServerToClientUsbfileReq packet;
                    auto data = packet.toString();
                    last_packet_ptr_ = &packet;
                    if (client->writeReady(data.get(), packet.DATA_SIZE + 8) < 0) {
                        logger_.fatal("write failed, errno code:" + std::to_string(errno));
                        break;
                    }
                    logger_.info("向已认证连接(IP=" + std::string(inet_ntoa(client->client_addr_))  + ")请求U盘文件信息");
                    logger_.info("(发送数据为:)");
                    logger_.print_data(data.get(), packet.DATA_SIZE+8);
                } while(0);  
            } else {
                do {
                    ServerToClientPrintportReq packet;
                    auto data = packet.toString();
                    last_packet_ptr_ = &packet;
                    if (client->writeReady(data.get(), packet.DATA_SIZE + 8) < 0) {
                        logger_.fatal("write failed, errno code:" + std::to_string(errno));
                        break;
                    }
                    logger_.info("向已认证连接(IP=" + std::string(inet_ntoa(client->client_addr_))  + ")请求打印口信息");
                    logger_.info("(发送数据为:)");
                    logger_.print_data(data.get(), packet.DATA_SIZE+8);
                } while(0);  
            }

            break;
        }
        case FunctionType::kUsbfileRsp: {
            ClientToServerUsbfileRsp packet(ph);
            packet.parse(client->buffer_+8);

            logger_.info("收到U盘文件列表信息!(IP=" + std::string(inet_ntoa(client->client_addr_)) + ").");
            logger_.info("(收到数据为:)");
            logger_.info(packet.info_);   

            do {
                ServerToClientPrintportReq packet;
                auto data = packet.toString();
                last_packet_ptr_ = &packet;
                if (client->writeReady(data.get(), packet.DATA_SIZE + 8) < 0) {
                    logger_.fatal("write failed, errno code:" + std::to_string(errno));
                    break;
                }
                logger_.info("向已认证连接(IP=" + std::string(inet_ntoa(client->client_addr_))  + ")请求打印口信息");
                logger_.info("(发送数据为:)");
                logger_.print_data(data.get(), packet.DATA_SIZE+8);
            } while(0); 

            break;
        }
        case FunctionType::kPrintportRsp: {
            ClientToServerPrintportRsp packet(ph);
            packet.parse(client->buffer_+8);

            logger_.info("收到打印口信息!(IP=" + std::string(inet_ntoa(client->client_addr_)) + ").");
            logger_.info("(收到数据为:)");
            logger_.info((packet.is_on_==0?std::string("服务已启动"):std::string("服务未启动")) + 
                    " 现有任务数" + std::to_string(packet.tasks_) + 
                    " 打印机名称" + std::string(packet.printer_name_));

            // TODO 数据库

            // 取打印任务信息
            if (packet.tasks_ > 0) {
                do {
                    ServerToClientPrintqueueReq packet;
                    auto data = packet.toString();
                    last_packet_ptr_ = &packet;
                    if (client->writeReady(data.get(), packet.DATA_SIZE + 8) < 0) {
                        logger_.fatal("write failed, errno code:" + std::to_string(errno));
                        break;
                    }
                    logger_.info("向已认证连接(IP=" + std::string(inet_ntoa(client->client_addr_))  + ")请求打印任务队列信息");
                    logger_.info("(发送数据为:)");
                    logger_.print_data(data.get(), packet.DATA_SIZE+8);
                } while(0); 
            } else {
                do {
                    ServerToClientEndinfoReq packet;
                    auto data = packet.toString();
                    last_packet_ptr_ = &packet;
                    if (client->writeReady(data.get(), packet.DATA_SIZE + 8) < 0) {
                        logger_.fatal("write failed, errno code:" + std::to_string(errno));
                        break;
                    }
                    logger_.info("向已认证连接(IP=" + std::string(inet_ntoa(client->client_addr_))  + ")请求终端服务信息");
                    logger_.info("(发送数据为:)");
                    logger_.print_data(data.get(), packet.DATA_SIZE+8);
                } while(0); 
            }

            break;
        }
        case FunctionType::kPrintqueueRsp: {
            ClientToServerPrintqueueRsp packet(ph);
            packet.parse(client->buffer_+8);


            logger_.info("收到打印队列信息!(IP=" + std::string(inet_ntoa(client->client_addr_)) + ").");
            logger_.info("(收到数据为:)");
            logger_.info(packet.info_);   

            // TODO 写入数据库

            // 5. 取以太口信息

            do {
                ServerToClientEndinfoReq packet;
                auto data = packet.toString();
                last_packet_ptr_ = &packet;
                if (client->writeReady(data.get(), packet.DATA_SIZE + 8) < 0) {
                    logger_.fatal("write failed, errno code:" + std::to_string(errno));
                    break;
                }
                logger_.info("向已认证连接(IP=" + std::string(inet_ntoa(client->client_addr_))  + ")请求终端服务信息");
                logger_.info("(发送数据为:)");
                logger_.print_data(data.get(), packet.DATA_SIZE+8);
            } while(0);
            break;
        }
        case FunctionType::kEndinfoRsp: {
            ClientToServerEndinfoRsp packet(ph);
            packet.parse(client->buffer_+8);

            int dumb_num = 0;
            for (int i = 0;i < client->async_port_num_; ++i) {
                if (packet.dump_end_[i] == 0x01) {
                    dumb_num++;
                    logger_.info("哑终端" + std::to_string(i));
                    client->dumb_queue.push(i+1);
                }
            }

            int ip_num = 0;
            for (int i = 0;i < 254; ++i) {
                if (packet.ip_end_[i] == 0x01) {
                    ip_num++;
                    logger_.info("IP终端" + std::to_string(i));
                    client->ip_queue.push(i+1);
                }
            }

            logger_.info("配置终端:" + std::to_string(packet.end_num_));

            if (!client->dumb_queue.empty()) {
                do {
                    uint16_t number = client->dumb_queue.front();
                    client->dumb_queue.pop();

                    ServerToClientDumbendReq packet(number);
                    auto data = packet.toString();
                    last_packet_ptr_ = &packet;
                    if (client->writeReady(data.get(), packet.DATA_SIZE + 8) < 0) {
                        logger_.fatal("write failed, errno code:" + std::to_string(errno));
                        break;
                    }
                    logger_.info("向已认证连接(IP=" + std::string(inet_ntoa(client->client_addr_))  + ")请求dumb" + std::to_string(number) + ".!");
                    logger_.info("(发送数据为:)");
                    logger_.print_data(data.get(), packet.DATA_SIZE+8);
                } while(0);
            } else if (!client->ip_queue.empty()) {
                do {
                    uint16_t number = client->ip_queue.front();
                    client->ip_queue.pop();

                    ServerToClientIpendReq packet(number);
                    auto data = packet.toString();
                    last_packet_ptr_ = &packet;
                    if (client->writeReady(data.get(), packet.DATA_SIZE + 8) < 0) {
                        logger_.fatal("write failed, errno code:" + std::to_string(errno));
                        break;
                    }
                    logger_.info("向已认证连接(IP=" + std::string(inet_ntoa(client->client_addr_))  + ")请求IP" + std::to_string(number) + ".!");
                    logger_.info("(发送数据为:)");
                    logger_.print_data(data.get(), packet.DATA_SIZE+8);
                } while(0);
            } else {
                do {
                    ServerToClientRcvAllAck packet;
                    auto data = packet.toString();
                    last_packet_ptr_ = &packet;
                    if (client->writeReady(data.get(), packet.DATA_SIZE + 8) < 0) {
                        logger_.fatal("write failed, errno code:" + std::to_string(errno));
                        break;
                    }
                    logger_.info("向已认证连接(IP=" + std::string(inet_ntoa(client->client_addr_))  + ")所有包均收到!");
                    logger_.info("(发送数据为:)");
                    logger_.print_data(data.get(), packet.DATA_SIZE+8);
                } while(0);
            }

            // TODO 写数据库
            
            // sprintf(sql_buffer, "insert into devstate_ttyinfo(devstate_ttyinfo_devid, devstate_ttyinfo_devno, \
            //     devstate_ttyinfo_ttyno, devstate_ttyinfo_readno) values('%d', '%d', '%d', '%d')",
            //     packet.dev_id_, 1, inet_ntoa(client->client_addr_), packet.group_serial_num_, packet.internal_serial_num_, 
            //     packet.device_model_
            // );
            // sqls.push_back(sql_buffer);

            break;
        }
        case FunctionType::kDumbendRsp: {
            ClientToServerDumbendRsp packet(ph);
            packet.parse(client->buffer_+8);

            logger_.info("收到dumb" + std::to_string(packet.packet_head_.get_number()) + "信息");
            logger_.info("(收到数据为:)");
            in_addr inaddr;
            inaddr.s_addr = packet.ip;
            logger_.info("终端号:" + std::to_string(packet.port) + " 当前虚屏:" + std::to_string(packet.active_vs) + 
                " 虚屏数量:" + std::to_string(packet.total_vs) + "\t类型" + std::string(packet.type) + 
                " 状态:" + std::string(packet.state) + " IP:" + std::string(inet_ntoa(inaddr)));


            char cur_time[80];
            time_t t = time(0);
            struct tm *p = gmtime(&t);
            strftime(cur_time, 80, "%Y-%m-%d %H:%M:%S", p);

            sprintf(sql_buffer, "insert into devstate_ttyinfo(devstate_ttyinfo_devid, devstate_ttyinfo_devno, devstate_ttyinfo_ttyno, \
                devstate_ttyinfo_time, devstate_ttyinfo_readno, devstate_ttyinfo_type, devstate_ttyinfo_state, \
                devstate_ttyinfo_ttyip, devstate_ttyinfo_scrnum) values('%u', '%d', %d, '%s', %d, '%s', '%s', '%s', %d)", 
                client->dev_id_, 1, packet.packet_head_.get_number()+900, cur_time, packet.port, packet.type, packet.state, 
                inet_ntoa(inaddr), packet.total_vs
                );
            sqls.push_back(sql_buffer);


            // 打印虚屏信息
            for (int i = 0;i < packet.total_vs; ++i) {
                in_addr inaddr;
                inaddr.s_addr = packet.vs[i].ip;
                logger_.info("  --虚屏号:" + std::to_string(packet.vs[i].no) + " 连接时间:" + std::to_string(packet.vs[i].time) + 
                " 提示串:" + std::string(packet.vs[i].hint) + "\n 服务器IP:" + std::string(inet_ntoa(inaddr)) + 
                " 端口:" + std::to_string(packet.vs[i].tcp_port) + " 协议:" + std::string(packet.vs[i].proto) + 
                " 状态:" + std::string(packet.vs[i].state) + " 类型:" + std::string(packet.vs[i].type) + 
                " 发终端:" + std::to_string(packet.vs[i].snd_end_bytes) + " 收终端:" + std::to_string(packet.vs[i].rcv_end_bytes) + 
                " 发服务器:" + std::to_string(packet.vs[i].snd_remote_bytes) + " 收服务器:" + std::to_string(packet.vs[i].snd_remote_bytes) + "\n" + 
                " min_ping:" + std::to_string(packet.vs[i].min_ping) + " avg_ping:" + std::to_string(packet.vs[i].avg_ping) + 
                " max_ping:" + std::to_string(packet.vs[i].max_ping));

                char cur_time[80];
                time_t t = time(0);
                struct tm *p = gmtime(&t);
                strftime(cur_time, 80, "%Y-%m-%d %H:%M:%S", p);
                sprintf(sql_buffer, "insert into devstate_scrinfo(devstate_scrinfo_devid, devstate_scrinfo_devno, \
                devstate_scrinfo_ttyno, devstate_scrinfo_scrno, devstate_scrinfo_time, devstate_scrinfo_is_current, \
                devstate_scrinfo_protocol, devstate_scrinfo_serverip, devstate_scrinfo_serverport, \
                devstate_scrinfo_state, devstate_scrinfo_ttytype, devstate_scrinfo_tx_server, devstate_scrinfo_rx_server, \
                devstate_scrinfo_tx_terminal, devstate_scrinfo_rx_terminal, devstate_scrinfo_ping_min, devstate_scrinfo_ping_avg, \
                devstate_scrinfo_ping_max) values('%u', '%d', %d, %d, '%s', '%s', '%s', '%s', %d, '%s', '%s', %d, %d, %d, %d, \
                %.1f, %.1f, %.1f) ", client->dev_id_, 1, packet.packet_head_.get_number()+900, packet.vs[i].no, cur_time,
                packet.active_vs == packet.vs[i].no?"*":"", packet.vs[i].proto, inet_ntoa(inaddr), packet.vs[i].tcp_port,
                packet.vs[i].state, packet.vs[i].type, packet.vs[i].snd_end_bytes, packet.vs[i].rcv_end_bytes, 
                packet.vs[i].snd_remote_bytes, packet.vs[i].rcv_remote_bytes, float(packet.vs[i].min_ping), float(packet.vs[i].avg_ping), 
                float(packet.vs[i].max_ping));
                sqls.push_back(sql_buffer);
            }

            // 继续发送
            if (!client->dumb_queue.empty()) {
                do {
                    uint16_t number = client->dumb_queue.front();
                    client->dumb_queue.pop();

                    ServerToClientDumbendReq packet(number);
                    auto data = packet.toString();
                    last_packet_ptr_ = &packet;
                    if (client->writeReady(data.get(), packet.DATA_SIZE + 8) < 0) {
                        logger_.fatal("write failed, errno code:" + std::to_string(errno));
                        break;
                    }
                    logger_.info("向已认证连接(IP=" + std::string(inet_ntoa(client->client_addr_))  + ")请求dumb" + std::to_string(number) + ".!");
                    logger_.info("(发送数据为:)");
                    logger_.print_data(data.get(), packet.DATA_SIZE+8);
                } while(0);
            } else if (!client->ip_queue.empty()) {
                do {
                    uint16_t number = client->ip_queue.front();
                    client->ip_queue.pop();

                    ServerToClientIpendReq packet(number);
                    auto data = packet.toString();
                    last_packet_ptr_ = &packet;
                    if (client->writeReady(data.get(), packet.DATA_SIZE + 8) < 0) {
                        logger_.fatal("write failed, errno code:" + std::to_string(errno));
                        break;
                    }
                    logger_.info("向已认证连接(IP=" + std::string(inet_ntoa(client->client_addr_))  + ")请求IP" + std::to_string(number) + ".!");
                    logger_.info("(发送数据为:)");
                    logger_.print_data(data.get(), packet.DATA_SIZE+8);
                } while(0);
            } else {
                do {
                    ServerToClientRcvAllAck packet;
                    auto data = packet.toString();
                    last_packet_ptr_ = &packet;
                    if (client->writeReady(data.get(), packet.DATA_SIZE + 8) < 0) {
                        logger_.fatal("write failed, errno code:" + std::to_string(errno));
                        break;
                    }
                    logger_.info("向已认证连接(IP=" + std::string(inet_ntoa(client->client_addr_))  + ")所有包均收到!");
                    logger_.info("(发送数据为:)");
                    logger_.print_data(data.get(), packet.DATA_SIZE+8);
                } while(0);
            }

            break;
        }
        case FunctionType::kIpendRsp: {
            ClientToServerIpendRsp packet(ph);
            packet.parse(client->buffer_+8);

            logger_.info("收到IP" + std::to_string(packet.packet_head_.get_number()) + "信息");
            logger_.info("(收到数据为:)");
            in_addr inaddr;
            inaddr.s_addr = packet.ip;
            logger_.info("终端号:" + std::to_string(packet.port) + " 当前虚屏:" + std::to_string(packet.active_vs) + 
                " 虚屏数量:" + std::to_string(packet.total_vs) + "\t类型" + std::string(packet.type) + 
                " 状态:" + std::string(packet.state) + " IP:" + std::string(inet_ntoa(inaddr)));


            char cur_time[80];
            time_t t = time(0);
            struct tm *p = gmtime(&t);
            strftime(cur_time, 80, "%Y-%m-%d %H:%M:%S", p);

            sprintf(sql_buffer, "insert into devstate_ttyinfo(devstate_ttyinfo_devid, devstate_ttyinfo_devno, devstate_ttyinfo_ttyno, \
                devstate_ttyinfo_time, devstate_ttyinfo_readno, devstate_ttyinfo_type, devstate_ttyinfo_state, \
                devstate_ttyinfo_ttyip, devstate_ttyinfo_scrnum) values('%u', '%d', %d, '%s', %d, '%s', '%s', '%s', %d)", 
                client->dev_id_, 1, packet.packet_head_.get_number(), cur_time, packet.port, packet.type, packet.state, 
                inet_ntoa(inaddr), packet.total_vs
                );
            sqls.push_back(sql_buffer);

            
            // 打印虚屏信息
            for (int i = 0;i < packet.total_vs; ++i) {
                in_addr inaddr;
                inaddr.s_addr = packet.vs[i].ip;
                logger_.info("  --虚屏号:" + std::to_string(packet.vs[i].no) + " 连接时间:" + std::to_string(packet.vs[i].time) + 
                " 提示串:" + std::string(packet.vs[i].hint) + "\n 服务器IP:" + std::string(inet_ntoa(inaddr)) + 
                " 端口:" + std::to_string(packet.vs[i].tcp_port) + " 协议:" + std::string(packet.vs[i].proto) + 
                " 状态:" + std::string(packet.vs[i].state) + " 类型:" + std::string(packet.vs[i].type) + 
                " 发终端:" + std::to_string(packet.vs[i].snd_end_bytes) + " 收终端:" + std::to_string(packet.vs[i].rcv_end_bytes) + 
                " 发服务器:" + std::to_string(packet.vs[i].snd_remote_bytes) + " 收服务器:" + std::to_string(packet.vs[i].snd_remote_bytes) + "\n" + 
                " min_ping:" + std::to_string(packet.vs[i].min_ping) + " avg_ping:" + std::to_string(packet.vs[i].avg_ping) + 
                " max_ping:" + std::to_string(packet.vs[i].max_ping));


                char cur_time[80];
                time_t t = time(0);
                struct tm *p = gmtime(&t);
                strftime(cur_time, 80, "%Y-%m-%d %H:%M:%S", p);
                sprintf(sql_buffer, "insert into devstate_scrinfo(devstate_scrinfo_devid, devstate_scrinfo_devno, \
                devstate_scrinfo_ttyno, devstate_scrinfo_scrno, devstate_scrinfo_time, devstate_scrinfo_is_current, \
                devstate_scrinfo_protocol, devstate_scrinfo_serverip, devstate_scrinfo_serverport, \
                devstate_scrinfo_state, devstate_scrinfo_ttytype, devstate_scrinfo_tx_server, devstate_scrinfo_rx_server, \
                devstate_scrinfo_tx_terminal, devstate_scrinfo_rx_terminal, devstate_scrinfo_ping_min, devstate_scrinfo_ping_avg, \
                devstate_scrinfo_ping_max) values('%u', '%d', %d, %d, '%s', '%s', '%s', '%s', %d, '%s', '%s', %d, %d, %d, %d, \
                %.1f, %.1f, %.1f) ", client->dev_id_, 1, packet.packet_head_.get_number(), packet.vs[i].no, cur_time,
                packet.active_vs == packet.vs[i].no?"*":"", packet.vs[i].proto, inet_ntoa(inaddr), packet.vs[i].tcp_port,
                packet.vs[i].state, packet.vs[i].type, packet.vs[i].snd_end_bytes, packet.vs[i].rcv_end_bytes, 
                packet.vs[i].snd_remote_bytes, packet.vs[i].rcv_remote_bytes, float(packet.vs[i].min_ping), float(packet.vs[i].avg_ping), 
                float(packet.vs[i].max_ping));
                sqls.push_back(sql_buffer);
            }

            // 继续发送
            if (!client->ip_queue.empty()) {
                do {
                    uint16_t number = client->ip_queue.front();
                    client->ip_queue.pop();

                    ServerToClientIpendReq packet(number);
                    auto data = packet.toString();
                    last_packet_ptr_ = &packet;
                    if (client->writeReady(data.get(), packet.DATA_SIZE + 8) < 0) {
                        logger_.fatal("write failed, errno code:" + std::to_string(errno));
                        break;
                    }
                    logger_.info("向已认证连接(IP=" + std::string(inet_ntoa(client->client_addr_))  + ")请求IP" + std::to_string(number) + ".!");
                    logger_.info("(发送数据为:)");
                    logger_.print_data(data.get(), packet.DATA_SIZE+8);
                } while(0);
            } else {
                do {
                    ServerToClientRcvAllAck packet;
                    auto data = packet.toString();
                    last_packet_ptr_ = &packet;
                    if (client->writeReady(data.get(), packet.DATA_SIZE + 8) < 0) {
                        logger_.fatal("write failed, errno code:" + std::to_string(errno));
                        break;
                    }
                    logger_.info("向已认证连接(IP=" + std::string(inet_ntoa(client->client_addr_))  + ")所有包均收到!");
                    logger_.info("(发送数据为:)");
                    logger_.print_data(data.get(), packet.DATA_SIZE+8);
                } while(0);
            }

            break;
        }
        case FunctionType::kSndAllAck: {
            logger_.info("已认证连接应答(IP=" + std::string(inet_ntoa(client->client_addr_))  + ")所有包均收到!");
            logger_.info("关闭与(IP=" + std::string(inet_ntoa(client->client_addr_))  + ")的TCP连接!");
            close(client->client_fd_);
            clients_.erase(client->client_fd_);
            open_file_cnt--;
            done_cnt++;
            break;
        }
        default: {
            logger_.fatal("报文类型错误! 当前open_file_cnt:" + std::to_string(open_file_cnt) + " done_cnt:" + std::to_string(done_cnt));
            logger_.fatal("错误报文类型:" + std::to_string(ph.get_function_type()));
            logger_.flush();
            // 尝试重发上一个报文...
            logger_.info("重发上一个报文...");
            do {
                auto data = last_packet_ptr_->toString();
                if (client->writeReady(data.get(), 8) < 0) {
                    logger_.fatal("write failed, errno code:" + std::to_string(errno));
                    break;
                }
            } while(0);
            break;
        }
        }
    }

}

