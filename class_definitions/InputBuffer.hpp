#pragma once

#include <iostream>
#include <string>

class InputBuffer {
private:
    std::string buffer;

public:
    InputBuffer() = default;

    static auto print_welcome_message() -> void {
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
