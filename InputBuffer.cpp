#include "types/enums.h"
#include <iostream>
#include <memory>
#include <string>

class InputBuffer {
private:
    std::string buffer;

public:
    InputBuffer() = default;

    static void print_prompt() {
        std::cout << "db > ";
    }

    void read_input() {
        std::getline(std::cin, buffer);

        if (buffer.empty()) {
            std::cerr << "Error reading input. Goodbye!" << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    [[nodiscard]] std::string get_buffer() const {
        return buffer;
    }
};

auto main() -> int {
    const auto input_buffer = std::make_unique<InputBuffer>();

    while (true) {
        InputBuffer::print_prompt();
        input_buffer->read_input();

        if (std::string command = input_buffer->get_buffer(); command == ".exit") {
            std::cout << "Exiting database." << std::endl;
            break;
        }
        else {
            std::cout << "Unrecognizable command " << "'" << command << "'" << std::endl;
        }

    }
}