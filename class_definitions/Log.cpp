//
// Created by acruse on 24/12/2024.
//

#include "Log.hpp"

#include <iostream>
#include <sstream>

Log::Log() {
    log_file.open(log_file_name, std::ios::app);
    if (!log_file.is_open()) {
        throw std::runtime_error("Log file could not be opened");
    }
}

Log::~Log() {
    if (log_file.is_open()) {
        log_file.close();
    }
}


auto Log::get_current_date() -> std::string {
    const auto now = std::chrono::system_clock::now();
    const auto date = std::chrono::system_clock::to_time_t(now);

    std::ostringstream oss;
    oss << std::put_time(std::localtime(&date), "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

auto Log::append_command(const std::string &command) {
    if (!log_file.is_open()) {
        throw std::runtime_error("Failed to open log file");
    }

    log_file << "[" << get_current_date() << "] " << command << std::endl;
    log_file.flush();
}


