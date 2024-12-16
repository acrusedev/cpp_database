#include "class_definitions/InputBuffer.hpp"
#include "handlers/MetaCommandHandler.hpp"
#include "types/enums.hpp"

auto main() -> int {
    const auto input_buffer = std::make_unique<InputBuffer>();

    while (true) {
        InputBuffer::print_welcome_message();
        input_buffer->read_input();

        if (input_buffer->get_buffer_first_char() == '.') {
            switch (exec_meta_command(input_buffer)) {
            case MetaCommandResults::SUCCESS:
                continue;
            case MetaCommandResults::UNRECOGNIZED_COMMAND:
                std::cout << "Unrecognizable META command " << input_buffer->get_buffer() << "\n";
                break;
            }
        }
        else {
            std::cout << "Unrecognizable QUERY command: '" << input_buffer->get_buffer() << "'" << std::endl;
        }
    }
}
