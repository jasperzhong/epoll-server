#ifndef LOGGER_H_
#define LOGGER_H_

#include <fstream>
#include <string>
#include <ctime>
#include <iomanip>
#include <iostream>

class Logger {
public:
    Logger() {}

    ~Logger();

    void start();

    void info(const std::string& msg);
    void warning(const std::string& msg);
    void fatal(const std::string& msg);

    void print_data(uint8_t*const begin, int n_bytes);

    void flush();
private:
    std::ofstream log_;
    char log_time[80];
};

#endif // !LOGGER_H_