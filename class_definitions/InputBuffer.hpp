#pragma once

#include <iostream>
#include <string>

class InputBuffer {
private:
    std::string buffer;

public:
    InputBuffer() = default;

    static auto print_welcome_message() -> void {
        std::cout << "Example supported SQL queries: \n";
        std::cout << "CREATE TABLE users (id INTEGER PRIMARY KEY, name TEXT)\n";
        std::cout << "INSERT INTO users (1, Jan)\n";
        std::cout << "SELECT */id/name/**/* FROM users\n";
        std::cout << "DROP TABLE users\n";
    }

    static auto print_ready_query() -> void {
        std::cout << "db > " << std::flush;
    }

    auto read_input() -> void {
        std::getline(std::cin, buffer);
    }

    [[nodiscard]] auto get_buffer() const -> std::string {
        return buffer;
    };

    [[nodiscard]] auto get_buffer_first_char() const -> char {
        if (buffer.empty()) {
            return '\0';
        }
        return buffer[0];
    };

    [[nodiscard]] auto is_input_empty() const -> bool {
        return buffer.empty();
    }
};
