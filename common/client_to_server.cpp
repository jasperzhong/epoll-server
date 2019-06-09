#include "client_to_server.h"

/* ClientToServerMinVersion */

ClientToServerMinVersion::ClientToServerMinVersion()
    : ClientToServerBase(FunctionType::kMinVersion, DATA_SIZE) {
    // TODO 暂时不初始化
}

bytePtr ClientToServerMinVersion::toString() const {
    return ClientToServerBase::toString(DATA_SIZE);
}

void ClientToServerMinVersion::filling(uint8_t*const begin) const {
    uint16_t net_0_1 = htons(min_main_version_num_);
    uint8_t net_2 = second1_version_num_;
    uint8_t net_3 = second2_version_num_;

    memcpy(begin, reinterpret_cast<uint8_t*>(&net_0_1), 2);
    memcpy(begin+2, reinterpret_cast<uint8_t*>(&net_2), 1);
    memcpy(begin+3, reinterpret_cast<uint8_t*>(&net_3), 1);
}

void ClientToServerMinVersion::parse(uint8_t*const begin) {
    min_main_version_num_ = ntohs(*reinterpret_cast<uint16_t*>(begin));
    second1_version_num_ = *reinterpret_cast<uint8_t*>(begin+2);
    second1_version_num_ = *reinterpret_cast<uint8_t*>(begin+3);
}


/* ClientToServerAuthRsp */

bytePtr ClientToServerAuthRsp::toString() const {
    return ClientToServerBase::toString(DATA_SIZE);
}

void ClientToServerAuthRsp::filling(uint8_t*const begin) const {
    uint16_t net_0_1 = htons(cpu_mhz_);
    uint16_t net_2_3 = htons(mem_total_);
    uint16_t net_4_5 = htons(flash_size_);
    uint16_t net_6_7 = htons(internal_serial_num_);
    // net_8_23 group_serial_num 
    // net_24_39 device_model
    // net_40_55 software_version
    uint8_t net_56 = eth_num_;
    uint8_t net_57 = sync_num_;
    uint8_t net_58 = asyn_num_;
    uint8_t net_59 = exch_num_;
    uint8_t net_60 = usb_num_;
    uint8_t net_61 = print_num_;
    uint16_t net_62_63 = 0;
    uint32_t net_64_67 = htonl(dev_id_);
    uint8_t net_68 = 1;
    uint8_t net_69 = 0;
    uint16_t net_70_71 = 0;
    // net_72_103 auth_keys
    uint32_t net_104_107 = htonl(random_num_);

    memcpy(begin, reinterpret_cast<uint8_t*>(&net_0_1), 2);
    memcpy(begin+2, reinterpret_cast<uint8_t*>(&net_2_3), 2);
    memcpy(begin+4, reinterpret_cast<uint8_t*>(&net_4_5), 2);
    memcpy(begin+6, reinterpret_cast<uint8_t*>(&net_6_7), 2);
    memcpy(begin+8, group_serial_num_, 16);
    memcpy(begin+24, device_model_, 16);
    memcpy(begin+40, software_version_, 16);
    memcpy(begin+56, reinterpret_cast<uint8_t*>(&net_56), 1);
    memcpy(begin+57, reinterpret_cast<uint8_t*>(&net_57), 1);
    memcpy(begin+58, reinterpret_cast<uint8_t*>(&net_58), 1);
    memcpy(begin+59, reinterpret_cast<uint8_t*>(&net_59), 1);
    memcpy(begin+60, reinterpret_cast<uint8_t*>(&net_60), 1);
    memcpy(begin+61, reinterpret_cast<uint8_t*>(&net_61), 1);
    memcpy(begin+62, reinterpret_cast<uint8_t*>(&net_62_63), 2);
    memcpy(begin+64, reinterpret_cast<uint8_t*>(&net_64_67), 4);
    memcpy(begin+68, reinterpret_cast<uint8_t*>(&net_68), 1);
    memcpy(begin+69, reinterpret_cast<uint8_t*>(&net_69), 1);
    memcpy(begin+70, reinterpret_cast<uint8_t*>(&net_70_71), 2);
    memcpy(begin+72, auth_keys, 32);
    memcpy(begin+104, reinterpret_cast<uint8_t*>(&net_104_107), 4);
}

void ClientToServerAuthRsp::parse(uint8_t*const begin) {
    cpu_mhz_ = ntohs(*reinterpret_cast<uint16_t*>(begin));
    mem_total_ = ntohs(*reinterpret_cast<uint16_t*>(begin+2));
    flash_size_ = ntohs(*reinterpret_cast<uint16_t*>(begin+4));
    internal_serial_num_ = ntohs(*reinterpret_cast<uint16_t*>(begin+6));
    memcpy(group_serial_num_, begin+8, 16);
    memcpy(device_model_, begin+24, 16);
    memcpy(software_version_, begin+40, 16);
    eth_num_ = *reinterpret_cast<uint8_t*>(begin+56);
    sync_num_ = *reinterpret_cast<uint8_t*>(begin+57);
    asyn_num_ = *reinterpret_cast<uint8_t*>(begin+58);
    exch_num_ = *reinterpret_cast<uint8_t*>(begin+59);
    usb_num_ = *reinterpret_cast<uint8_t*>(begin+60);
    print_num_ = *reinterpret_cast<uint8_t*>(begin+61);
    dev_id_ = ntohl(*reinterpret_cast<uint32_t*>(begin+64));
    memcpy(auth_keys, begin+72, 32);
    random_num_ = ntohl(*reinterpret_cast<uint32_t*>(begin+104));
}

void ClientToServerAuthRsp::decrypt(uint8_t*const begin) {
    cpu_mhz_ = (*reinterpret_cast<uint16_t*>(begin));
    mem_total_ = (*reinterpret_cast<uint16_t*>(begin+2));
    flash_size_ = (*reinterpret_cast<uint16_t*>(begin+4));
    internal_serial_num_ = (*reinterpret_cast<uint16_t*>(begin+6));
    memcpy(group_serial_num_, begin+8, 16);
    memcpy(device_model_, begin+24, 16);
    memcpy(software_version_, begin+40, 16);
    eth_num_ = *reinterpret_cast<uint8_t*>(begin+56);
    sync_num_ = *reinterpret_cast<uint8_t*>(begin+57);
    asyn_num_ = *reinterpret_cast<uint8_t*>(begin+58);
    exch_num_ = *reinterpret_cast<uint8_t*>(begin+59);
    usb_num_ = *reinterpret_cast<uint8_t*>(begin+60);
    print_num_ = *reinterpret_cast<uint8_t*>(begin+61);
    dev_id_ = (*reinterpret_cast<uint32_t*>(begin+64));
    memcpy(auth_keys, begin+72, 32);
    random_num_ = (*reinterpret_cast<uint32_t*>(begin+104));
}

/* ClientToServerSysRsp */

bytePtr ClientToServerSysRsp::toString() const {
    return ClientToServerBase::toString(DATA_SIZE);
}

void ClientToServerSysRsp::filling(uint8_t*const begin) const {
    uint32_t net_0_3 = htonl(user_cpu_time_);
    uint32_t net_4_7 = htonl(nice_cpu_time_);
    uint32_t net_8_11 = htonl(sys_cpu_time_);
    uint32_t net_12_15 = htonl(idle_cpu_time_);
    uint32_t net_16_19 = htonl(freed_mem_);

    memcpy(begin, reinterpret_cast<uint8_t*>(&net_0_3), 4);
    memcpy(begin+4, reinterpret_cast<uint8_t*>(&net_4_7), 4);
    memcpy(begin+8, reinterpret_cast<uint8_t*>(&net_8_11), 4);
    memcpy(begin+12, reinterpret_cast<uint8_t*>(&net_12_15), 4);
    memcpy(begin+16, reinterpret_cast<uint8_t*>(&net_16_19), 4);
}

void ClientToServerSysRsp::parse(uint8_t*const begin) {
    user_cpu_time_ = ntohl(*reinterpret_cast<uint32_t*>(begin));
    nice_cpu_time_ = ntohl(*reinterpret_cast<uint32_t*>(begin+4));
    sys_cpu_time_ = ntohl(*reinterpret_cast<uint32_t*>(begin+8));
    idle_cpu_time_ = ntohl(*reinterpret_cast<uint32_t*>(begin+12));
    freed_mem_ = ntohl(*reinterpret_cast<uint32_t*>(begin+16));
}


/* ClientToServerConfRsp */

void ClientToServerConfRsp::parse(uint8_t*const begin) {
    conf_.append(reinterpret_cast<char*>(begin), packet_head_.get_data_length());
}


/* ClientToServerProcRsp */

void ClientToServerProcRsp::parse(uint8_t*const begin) {
    info_.append(reinterpret_cast<char*>(begin), packet_head_.get_data_length());
}


/* ClientToServerEthRsp */

bytePtr ClientToServerEthRsp::toString() const {
    return ClientToServerBase::toString(DATA_SIZE);
}

void ClientToServerEthRsp::filling(uint8_t*const begin) const {
    uint8_t net_0  = is_exist_;
    uint8_t net_1  = is_config_;
    uint8_t net_2  = up_down_;
    uint8_t net_3  = 0;
    uint8_t net_4  = mac_0_;
    uint8_t net_5  = mac_1_;
    uint8_t net_6  = mac_2_;
    uint8_t net_7  = mac_3_;
    uint8_t net_8  = mac_4_;
    uint8_t net_9  = mac_5_;
    uint16_t net_10_11 = htons(opt_);

    uint32_t net_12_15 = htonl(ip_);
    uint32_t net_16_19 = htonl(mask_);
    uint32_t net_20_23 = htonl(ip_1_);
    uint32_t net_24_27 = htonl(mask_1_);
    uint32_t net_28_31 = htonl(ip_2_);
    uint32_t net_32_35 = htonl(mask_2_);
    uint32_t net_36_39 = htonl(ip_3_);
    uint32_t net_40_43 = htonl(mask_3_);
    uint32_t net_44_47 = htonl(ip_4_);
    uint32_t net_48_51 = htonl(mask_4_);
    uint32_t net_52_55 = htonl(ip_5_);
    uint32_t net_56_59 = htonl(mask_5_);

    uint32_t net_60_63 = htonl(rcv_bytes_);
    uint32_t net_64_67 = htonl(rcv_packets_);
    uint32_t net_68_71 = htonl(rcv_err_packets_);
    uint32_t net_72_75 = htonl(rcv_drop_packets_);
    uint32_t net_76_79 = htonl(rcv_fifo_packets_);
    uint32_t net_80_83 = htonl(rcv_frames_);
    uint32_t net_84_87 = htonl(rcv_compress_packets_);
    uint32_t net_88_91 = htonl(rcv_broadcast_packets_);

    uint32_t net_92_95 = htonl(snd_bytes_);
    uint32_t net_96_99 = htonl(snd_packets_);
    uint32_t net_100_103 = htonl(snd_err_packets_);
    uint32_t net_104_107 = htonl(snd_drop_packets_);
    uint32_t net_108_111 = htonl(snd_fifo_packets_);
    uint32_t net_112_115 = htonl(snd_frames_);
    uint32_t net_116_119 = htonl(snd_compress_packets_);
    uint32_t net_120_123 = htonl(snd_broadcast_packets_);

    memcpy(begin, reinterpret_cast<uint8_t*>(&net_0), 1);
    memcpy(begin+1, reinterpret_cast<uint8_t*>(&net_1), 1);
    memcpy(begin+2, reinterpret_cast<uint8_t*>(&net_2), 1);
    memcpy(begin+3, reinterpret_cast<uint8_t*>(&net_3), 1);
    memcpy(begin+4, reinterpret_cast<uint8_t*>(&net_4), 1);
    memcpy(begin+5, reinterpret_cast<uint8_t*>(&net_5), 1);
    memcpy(begin+6, reinterpret_cast<uint8_t*>(&net_6), 1);
    memcpy(begin+7, reinterpret_cast<uint8_t*>(&net_7), 1);
    memcpy(begin+8, reinterpret_cast<uint8_t*>(&net_8), 1);
    memcpy(begin+9, reinterpret_cast<uint8_t*>(&net_9), 1);
    memcpy(begin+10, reinterpret_cast<uint8_t*>(&net_10_11), 2);

    memcpy(begin+12, reinterpret_cast<uint8_t*>(&net_12_15), 4);
    memcpy(begin+16, reinterpret_cast<uint8_t*>(&net_16_19), 4);
    memcpy(begin+20, reinterpret_cast<uint8_t*>(&net_20_23), 4);
    memcpy(begin+24, reinterpret_cast<uint8_t*>(&net_24_27), 4);
    memcpy(begin+28, reinterpret_cast<uint8_t*>(&net_28_31), 4);
    memcpy(begin+32, reinterpret_cast<uint8_t*>(&net_32_35), 4);
    memcpy(begin+36, reinterpret_cast<uint8_t*>(&net_36_39), 4);
    memcpy(begin+40, reinterpret_cast<uint8_t*>(&net_40_43), 4);
    memcpy(begin+44, reinterpret_cast<uint8_t*>(&net_44_47), 4);
    memcpy(begin+48, reinterpret_cast<uint8_t*>(&net_48_51), 4);
    memcpy(begin+52, reinterpret_cast<uint8_t*>(&net_52_55), 4);
    memcpy(begin+56, reinterpret_cast<uint8_t*>(&net_56_59), 4);

    memcpy(begin+60, reinterpret_cast<uint8_t*>(&net_60_63), 4);
    memcpy(begin+64, reinterpret_cast<uint8_t*>(&net_64_67), 4);
    memcpy(begin+68, reinterpret_cast<uint8_t*>(&net_68_71), 4);
    memcpy(begin+72, reinterpret_cast<uint8_t*>(&net_72_75), 4);
    memcpy(begin+76, reinterpret_cast<uint8_t*>(&net_76_79), 4);
    memcpy(begin+80, reinterpret_cast<uint8_t*>(&net_80_83), 4);
    memcpy(begin+84, reinterpret_cast<uint8_t*>(&net_84_87), 4);
    memcpy(begin+88, reinterpret_cast<uint8_t*>(&net_88_91), 4);

    memcpy(begin+92, reinterpret_cast<uint8_t*>(&net_92_95), 4);
    memcpy(begin+96, reinterpret_cast<uint8_t*>(&net_96_99), 4);
    memcpy(begin+100, reinterpret_cast<uint8_t*>(&net_100_103), 4);
    memcpy(begin+104, reinterpret_cast<uint8_t*>(&net_104_107), 4);
    memcpy(begin+108, reinterpret_cast<uint8_t*>(&net_108_111), 4);
    memcpy(begin+112, reinterpret_cast<uint8_t*>(&net_112_115), 4);
    memcpy(begin+116, reinterpret_cast<uint8_t*>(&net_116_119), 4);
    memcpy(begin+120, reinterpret_cast<uint8_t*>(&net_120_123), 4);
}

void ClientToServerEthRsp::parse(uint8_t*const begin) {
    is_exist_ = *reinterpret_cast<uint8_t*>(begin);
    is_config_ = *reinterpret_cast<uint8_t*>(begin+1);
    up_down_ = *reinterpret_cast<uint8_t*>(begin+2);
    // pad
    mac_0_ = *reinterpret_cast<uint8_t*>(begin+4);
    mac_1_ = *reinterpret_cast<uint8_t*>(begin+5);
    mac_2_ = *reinterpret_cast<uint8_t*>(begin+6);
    mac_3_ = *reinterpret_cast<uint8_t*>(begin+7);
    mac_4_ = *reinterpret_cast<uint8_t*>(begin+8);
    mac_5_ = *reinterpret_cast<uint8_t*>(begin+9);
    opt_ = ntohs(*reinterpret_cast<uint16_t*>(begin+10));

    ip_ = ntohl(*reinterpret_cast<uint32_t*>(begin+12));
    mask_ = ntohl(*reinterpret_cast<uint32_t*>(begin+16));
    ip_1_ = ntohl(*reinterpret_cast<uint32_t*>(begin+20));
    mask_1_ = ntohl(*reinterpret_cast<uint32_t*>(begin+24));
    ip_2_ = ntohl(*reinterpret_cast<uint32_t*>(begin+28));
    mask_2_ = ntohl(*reinterpret_cast<uint32_t*>(begin+32));
    ip_3_ = ntohl(*reinterpret_cast<uint32_t*>(begin+36));
    mask_3_ = ntohl(*reinterpret_cast<uint32_t*>(begin+40));
    ip_4_ = ntohl(*reinterpret_cast<uint32_t*>(begin+44));
    mask_4_ = ntohl(*reinterpret_cast<uint32_t*>(begin+48));
    ip_5_ = ntohl(*reinterpret_cast<uint32_t*>(begin+52));
    mask_5_ = ntohl(*reinterpret_cast<uint32_t*>(begin+56));

    rcv_bytes_ = ntohl(*reinterpret_cast<uint32_t*>(begin+60));
    rcv_packets_ = ntohl(*reinterpret_cast<uint32_t*>(begin+64));
    rcv_err_packets_ = ntohl(*reinterpret_cast<uint32_t*>(begin+68));
    rcv_drop_packets_ = ntohl(*reinterpret_cast<uint32_t*>(begin+72));
    rcv_fifo_packets_ = ntohl(*reinterpret_cast<uint32_t*>(begin+76));
    rcv_frames_ = ntohl(*reinterpret_cast<uint32_t*>(begin+80));
    rcv_compress_packets_ = ntohl(*reinterpret_cast<uint32_t*>(begin+84));
    rcv_broadcast_packets_ = ntohl(*reinterpret_cast<uint32_t*>(begin+88));

    snd_bytes_ = ntohl(*reinterpret_cast<uint32_t*>(begin+92));
    snd_packets_ = ntohl(*reinterpret_cast<uint32_t*>(begin+96));
    snd_err_packets_ = ntohl(*reinterpret_cast<uint32_t*>(begin+100));
    snd_drop_packets_ = ntohl(*reinterpret_cast<uint32_t*>(begin+104));
    snd_fifo_packets_ = ntohl(*reinterpret_cast<uint32_t*>(begin+108));
    snd_frames_ = ntohl(*reinterpret_cast<uint32_t*>(begin+112));
    snd_compress_packets_ = ntohl(*reinterpret_cast<uint32_t*>(begin+116));
    snd_broadcast_packets_ = ntohl(*reinterpret_cast<uint32_t*>(begin+120));
}


/* ClientToServerUsbportRsp */

bytePtr ClientToServerUsbportRsp::toString() const {
    return ClientToServerBase::toString(DATA_SIZE);
}

void ClientToServerUsbportRsp::filling(uint8_t*const begin) const {
    uint8_t net_0 = is_plug_usb_;
    uint8_t net_1 = 0;
    uint8_t net_2 = 0;
    uint8_t net_3 = 0;
    memcpy(begin, reinterpret_cast<uint8_t*>(&net_0), 1);
    memcpy(begin+1, reinterpret_cast<uint8_t*>(&net_1), 1);
    memcpy(begin+2, reinterpret_cast<uint8_t*>(&net_2), 1);
    memcpy(begin+3, reinterpret_cast<uint8_t*>(&net_3), 1);
}

void ClientToServerUsbportRsp::parse(uint8_t*const begin) {
    is_plug_usb_ = *reinterpret_cast<uint8_t*>(begin);
}


/* ClientToServerUsbfileRsp */

void ClientToServerUsbfileRsp::parse(uint8_t*const begin) {
    info_.append(reinterpret_cast<char*>(begin), packet_head_.get_data_length());
}

/* ClientToServerPrintportRsp */

bytePtr ClientToServerPrintportRsp::toString() const {
    return ClientToServerBase::toString(DATA_SIZE);
}

void ClientToServerPrintportRsp::filling(uint8_t*const begin) const {
    uint8_t net_0 = is_on_;
    uint8_t net_1 = 0;
    uint16_t net_2_3 = htons(tasks_);
    // net_4_35 printer_name_
    memcpy(begin, reinterpret_cast<uint8_t*>(&net_0), 1);
    memcpy(begin+1, reinterpret_cast<uint8_t*>(&net_1), 1);
    memcpy(begin+2, reinterpret_cast<uint16_t*>(&net_2_3), 2);
    memcpy(begin+4, printer_name_, 32);
}

void ClientToServerPrintportRsp::parse(uint8_t*const begin) {
    is_on_ = *reinterpret_cast<uint8_t*>(begin);
    tasks_ = ntohs(*reinterpret_cast<uint16_t*>(begin+2));
    memcpy(printer_name_, begin+4, 32);
}

/* ClientToServerPrintqueueRsp */

void ClientToServerPrintqueueRsp::parse(uint8_t*const begin) {
    info_.append(reinterpret_cast<char*>(begin), packet_head_.get_data_length());
}

/* ClientToServerEndinfoRsp */

bytePtr ClientToServerEndinfoRsp::toString() const {
    return ClientToServerBase::toString(DATA_SIZE);
}

void ClientToServerEndinfoRsp::filling(uint8_t*const begin) const {
    uint8_t tmp = htons(end_num_);
    memcpy(begin, dump_end_, 16);
    memcpy(begin+16, ip_end_, 254);
    memcpy(begin+16+254, reinterpret_cast<uint8_t*>(&tmp), 2);
}

void ClientToServerEndinfoRsp::parse(uint8_t*const begin) {
    memcpy(dump_end_, begin, 16);
    memcpy(ip_end_, begin+16, 254);
    end_num_ = ntohs(*reinterpret_cast<uint16_t*>(begin+16+254));
}


/* ClientToServerDumbendRsp */

void ClientToServerDumbendRsp::parse(uint8_t*const begin) {
    port = *reinterpret_cast<uint8_t*>(begin);
    config_port = *reinterpret_cast<uint8_t*>(begin+1);
    active_vs = *reinterpret_cast<uint8_t*>(begin+2);
    total_vs = *reinterpret_cast<uint8_t*>(begin+3);
    ip = ntohl(*reinterpret_cast<uint32_t*>(begin+4));
    memcpy(type, begin+8, 12);
    memcpy(state, begin+20, 8);
    
    // parse vs
    for (int i = 0;i < total_vs; ++i) {
        parseVS(begin+28+24*4*i);
    }
}

void ClientToServerDumbendRsp::parseVS(uint8_t*const begin) {
    VirtualScreen v;
    v.no = *reinterpret_cast<uint8_t*>(begin);
    // pad
    v.tcp_port = ntohs(*reinterpret_cast<uint16_t*>(begin+2));
    v.ip = ntohl(*reinterpret_cast<uint32_t*>(begin+4));
    memcpy(v.proto, begin+8, 12);
    memcpy(v.state, begin+20, 12);
    memcpy(v.hint, begin+28, 24);
    memcpy(v.type, begin+52, 12);
    v.time = ntohl(*reinterpret_cast<uint32_t*>(begin+64));
    v.snd_end_bytes = ntohl(*reinterpret_cast<uint32_t*>(begin+68));
    v.rcv_end_bytes = ntohl(*reinterpret_cast<uint32_t*>(begin+72));
    v.snd_remote_bytes = ntohl(*reinterpret_cast<uint32_t*>(begin+76));
    v.rcv_remote_bytes = ntohl(*reinterpret_cast<uint32_t*>(begin+80));
    v.min_ping= ntohl(*reinterpret_cast<uint32_t*>(begin+84));
    v.avg_ping = ntohl(*reinterpret_cast<uint32_t*>(begin+88));
    v.max_ping = ntohl(*reinterpret_cast<uint32_t*>(begin+92));

    vs.push_back(v);
}


/* ClientToServerIpendRsp */

void ClientToServerIpendRsp::parse(uint8_t*const begin) {
    port = *reinterpret_cast<uint8_t*>(begin);
    config_port = *reinterpret_cast<uint8_t*>(begin+1);
    active_vs = *reinterpret_cast<uint8_t*>(begin+2);
    total_vs = *reinterpret_cast<uint8_t*>(begin+3);
    ip = ntohl(*reinterpret_cast<uint32_t*>(begin+4));
    memcpy(type, begin+8, 12);
    memcpy(state, begin+20, 8);
    
    // parse vs
    for (int i = 0;i < total_vs; ++i) {
        parseVS(begin+28+24*4*i);
    }
}

void ClientToServerIpendRsp::parseVS(uint8_t*const begin) {
    VirtualScreen v;
    v.no = *reinterpret_cast<uint8_t*>(begin);
    // pad
    v.tcp_port = ntohs(*reinterpret_cast<uint16_t*>(begin+2));
    v.ip = ntohl(*reinterpret_cast<uint32_t*>(begin+4));
    memcpy(v.proto, begin+8, 12);
    memcpy(v.state, begin+20, 12);
    memcpy(v.hint, begin+28, 24);
    memcpy(v.type, begin+52, 12);
    v.time = ntohl(*reinterpret_cast<uint32_t*>(begin+64));
    v.snd_end_bytes = ntohl(*reinterpret_cast<uint32_t*>(begin+68));
    v.rcv_end_bytes = ntohl(*reinterpret_cast<uint32_t*>(begin+72));
    v.snd_remote_bytes = ntohl(*reinterpret_cast<uint32_t*>(begin+76));
    v.rcv_remote_bytes = ntohl(*reinterpret_cast<uint32_t*>(begin+80));
    v.min_ping= ntohl(*reinterpret_cast<uint32_t*>(begin+84));
    v.avg_ping = ntohl(*reinterpret_cast<uint32_t*>(begin+88));
    v.max_ping = ntohl(*reinterpret_cast<uint32_t*>(begin+92));

    vs.push_back(v);
}