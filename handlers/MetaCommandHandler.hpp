#pragma once

#include <memory>
#include <string>
#include <iostream>
#include <cstdlib>

#include "../class_definitions/InputBuffer.hpp"
#include "../types/enums.hpp"

struct MetaCommandHandler {
	static auto exec_meta_command(const std::unique_ptr<InputBuffer>& input_buffer) -> MetaCommandResults {
	if (input_buffer -> get_buffer() == ".exit") {
		std::cout << "Meta command executed. Exiting database." << std::endl;
		exit(EXIT_SUCCESS);
	}

	return MetaCommandResults::UNRECOGNIZED_COMMAND;
	}
};
