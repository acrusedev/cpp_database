#include "class_definitions/InputBuffer.hpp"
#include "handlers/MetaCommandHandler.hpp"
#include "types/enums.hpp"

auto main() -> int {
    const auto input_buffer = std::make_unique<InputBuffer>();

    while (true) {
        std::cout << std::endl;
        InputBuffer::print_welcome_message();
        input_buffer->read_input();

        if (input_buffer->is_input_empty()) {
            std::cout << "Input buffer is empty. Please enter a valid command." << std::flush;
            continue;
        }

        if (input_buffer->get_buffer_first_char() == '.') {
            switch (exec_meta_command(input_buffer)) {
                case MetaCommandResults::SUCCESS:
                    continue;
                case MetaCommandResults::UNRECOGNIZED_COMMAND:
                    std::cout << "Unrecognizable META command: " << input_buffer->get_buffer() << std::endl;
                break;
            }
        } else {
            std::cout << "Unrecognizable QUERY command: '" << input_buffer->get_buffer() << "'" << std::endl;
        }
    }
}
