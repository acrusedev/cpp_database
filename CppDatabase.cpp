#include <iostream>
#include <string>
#include <memory>
#include "class_definitions/DatabasePersistence.hpp"
#include "class_definitions/SQLParser.hpp"

void print_welcome() {
    std::cout << "CppDatabase - Prosta baza danych SQL\n";
    std::cout << "Przykłady:\n";
    std::cout << "CREATE TABLE users (id INTEGER PRIMARY KEY, name TEXT)\n";
    std::cout << "INSERT INTO users (1, Jan)\n";
    std::cout << "SELECT * FROM users\n\n";
}

void print_tables(const DatabasePersistence& db) {
    auto tables = db.list_tables();
    if (tables.empty()) {
        std::cout << "Brak tabel w bazie danych.\n";
    } else {
        std::cout << "Dostępne tabele:\n";
        for (const auto& table : tables) {
            std::cout << "- " << table << "\n";
        }
    }
    std::cout << std::endl;
}

int main() {
    print_welcome();

    auto db = std::make_shared<DatabasePersistence>("./data");
    SQLParser parser(db);

    // Wyświetl dostępne tabele przy starcie
    print_tables(*db);

    std::string line;
    while (true) {
        std::cout << "sql> ";
        std::getline(std::cin, line);

        if (line.empty()) {
            continue;
        }

        if (line == "exit") {
            break;
        }

        try {
            parser.execute_query(line);
        } catch (const std::exception& e) {
            std::cout << "Błąd: " << e.what() << "\n";
            
            // Pokaż przykład użycia w zależności od komendy
            if (line.find("CREATE") != std::string::npos) {
                std::cout << "Przykład: CREATE TABLE users (id INTEGER PRIMARY KEY, name TEXT)\n";
            }
            else if (line.find("INSERT") != std::string::npos) {
                std::cout << "Przykład: INSERT INTO users (1, Jan)\n";
            }
            else if (line.find("SELECT") != std::string::npos) {
                std::cout << "Przykład: SELECT * FROM users\n";
            }
            else if (line.find("UPDATE") != std::string::npos) {
                std::cout << "Przykład: UPDATE users SET name = Jan WHERE id = 1\n";
            }
            else if (line.find("DELETE") != std::string::npos) {
                std::cout << "Przykład: DELETE FROM users WHERE id = 1\n";
            }
        }
    }

    return 0;
}
