#include "logger.h"

void Logger::start() {
    log_.open("yzmond.log");
    if (!log_.is_open()) {
        return;
    }
}


Logger::~Logger() {
    if (log_.is_open()) {
        log_.close();
    }
}

void Logger::info(const std::string& msg) {
    time_t t = time(0);
    struct tm *p = gmtime(&t);
    strftime(log_time, 80, "%Y-%m-%d %H:%M:%S", p);
    log_ << log_time << " [INFO] "  << msg << "\n";
}

void Logger::warning(const std::string& msg) {
    time_t t = time(0);
    struct tm *p = gmtime(&t);
    strftime(log_time, 80, "%Y-%m-%d %H:%M:%S", p);
    log_ << log_time << " [WARNING] "  << msg << "\n";
}


void Logger::fatal(const std::string& msg) {
    time_t t = time(0);
    struct tm *p = gmtime(&t);
    strftime(log_time, 80, "%Y-%m-%d %H:%M:%S", p);
    log_ << log_time << " [FATAL]"  << msg << "\n";
}


void Logger::print_data(uint8_t*const begin, int n_bytes) {
    int cnt = 0x00;
    int i;
    for (i = 0; i < n_bytes; ++i) {
        if (i % 16 == 0) {
            log_.fill('0');
            log_.width(4);
            log_ << cnt << ":  ";
            cnt += 0x10;
        } else if (i % 8 == 0) {
            log_ << "- ";
        }
        log_.fill('0');
        log_.width(2);
        log_ << std::hex << unsigned(begin[i]) << " ";
        if ((i+1) % 16 == 0) {
            log_ << " ";
            for (int j = i-15; j<=i; ++j) {
                if (begin[j] >= 0x20 && begin[j] <= 0x7E) {
                    log_ << char(begin[j]);
                } else {
                    log_ << ".";
                }
            }
            log_ << "\n";
        }
    }

    if ((i+1) % 16 != 0) {
        for (int j = (i+1)%16; j > 0; j--)
            log_ << " ";
        for (int j = i-15; j<=i; ++j) {
            if (begin[j] >= 0x20 && begin[j] <= 0x7E) {
                log_ << char(begin[j]);
            } else {
                log_ << ".";
            }
        }
        log_ << "\n";
    }
}

void Logger::flush() {
    log_.flush();
}