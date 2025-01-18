#include <iostream>
#include <string>
#include <memory>
#include "class_definitions/DatabasePersistence.hpp"
#include "handlers/SqlCommandHandler.hpp"
#include "class_definitions/InputBuffer.hpp"
#include "types/enums.hpp"
#include "handlers/MetaCommandHandler.hpp"


// BAZA DANYCH
// OBSLUGIWANE POLECENIA: DQL: SELECT * FROM TABLE, SELECT COL_1, COL_2 FROM TABLE, SELECT * FROM WHERE CONDITION, SELECT * FROM WHERE CONDITION_1 AND/OR CONDITION_2
//                        DDL: CREATE TABLE TABLE_NAME (COL_NAME, COL_TYPE(INTEGER, BOOL, TEXT), PRIMARY KEY, IS NULLABLE)
//                        DML: INSERT INTO TABLE_NAME (VALUE1, VALUE2, ...), UPDATE USERS SET NAME = JAN WHERE ID = 2




void print_tables(const DatabasePersistence& db) {
    auto tables = db.list_tables();
    if (tables.empty()) {
        std::cout << "No tables found.\n";
    } else {
        std::cout << "Found tables:\n";
        for (const auto& table : tables) {
            std::cout << "*" << table << "\n";
        }
    }
    std::cout << std::endl;
}

int main() {
    const auto input_buffer = std::make_unique<InputBuffer>();
    const auto db = std::make_shared<DatabasePersistence>("./data");
    auto sql_handler = SqlCommandHandler(db);

    print_tables(*db);

    InputBuffer::print_welcome_message();
    while (true) {
        InputBuffer::print_ready_query();
        input_buffer->read_input();

        if (input_buffer->is_input_empty()) {
            std::cout << "Input buffer is empty. Please enter a valid command." << std::flush;
            continue;
        }

        if (input_buffer->get_buffer_first_char() == '.') {
            switch (MetaCommandHandler::exec_meta_command(input_buffer)) {
                case MetaCommandResults::SUCCESS:
                    continue;
                case MetaCommandResults::UNRECOGNIZED_COMMAND:
                    std::cout << "ERROR: Unknown meta command " << input_buffer->get_buffer() << "\n";
                    continue;
            }
        }

        switch (sql_handler.exec_sql_command(input_buffer)) {
            case SqlCommandResults::SUCCESS:
                break;
            case SqlCommandResults::UNKNOWN_ERROR:
                std::cout << "ERROR: Unknown error occurred\n";
                break;
            case SqlCommandResults::UNKNOWN_COMMAND:
                std::cout << "ERROR: Unknown command " <<  input_buffer->get_buffer() << "\n";
                break;
            case SqlCommandResults::TABLE_NOT_FOUND:
                std::cout << "ERROR: Table not found\n";
                break;
            case SqlCommandResults::TABLE_ALREADY_EXISTS:
                std::cout << "ERROR: Table already exists\n";
                break;
            case SqlCommandResults::INCORRECT_EXPRESSION:
                std::cout << "ERROR: Incorrect SQL expression\n";
                break;
            case SqlCommandResults::EMPTY_QUERY:
                std::cout << "ERROR: Empty query\n";
                break;
            case SqlCommandResults::TABLE_DOES_NOT_EXIST:
                std::cout << "ERROR: Table does not exist\n";
                break;
            default: ;
        }
    }

    return 0;
}
