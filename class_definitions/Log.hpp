#ifndef LOG_H
#define LOG_H
#include <fstream>


class Log {
private:
    std::ofstream log_file;
    inline auto static log_file_name = "db.log";
    [[nodiscard]] static auto get_current_date() -> std::string;

public:
    Log();
    ~Log();

    auto append_command(const std::string &command);

    Log(const Log&) = delete;
    Log& operator=(const Log&) = delete;
};




#endif //LOG_H
