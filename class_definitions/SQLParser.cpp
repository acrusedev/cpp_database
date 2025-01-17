#include "SQLParser.hpp"
#include <sstream>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <stdexcept>

std::vector<std::string> SQLParser::tokenize(const std::string& query) {
    std::vector<std::string> tokens;
    std::istringstream iss(query);
    std::string token;

    while (iss >> token) {
        // Usuń przecinki i nawiasy
        token.erase(std::remove(token.begin(), token.end(), ','), token.end());
        token.erase(std::remove(token.begin(), token.end(), '('), token.end());
        token.erase(std::remove(token.begin(), token.end(), ')'), token.end());
        
        if (!token.empty()) {
            to_upper(token);
            tokens.push_back(token);
        }
    }
    return tokens;
}

void SQLParser::to_upper(std::string& str) {
    std::transform(str.begin(), str.end(), str.begin(), ::toupper);
}

void SQLParser::execute_query(const std::string& query) {
    auto tokens = tokenize(query);
    if (tokens.empty()) {
        throw std::runtime_error("Puste zapytanie");
    }

    const auto& command = tokens[0];
    if (command == "CREATE") {
        handle_create_table(tokens);
    } else if (command == "INSERT") {
        handle_insert(tokens);
    } else if (command == "SELECT") {
        handle_select(tokens);
    } else if (command == "UPDATE") {
        handle_update(tokens);
    } else if (command == "DELETE") {
        handle_delete(tokens);
    } else if (command == "DROP") {
        handle_drop_table(tokens);
    } else {
        throw std::runtime_error("Nieznana komenda: " + command);
    }
}

void SQLParser::handle_create_table(const std::vector<std::string>& tokens) {
    if (tokens.size() < 4 || tokens[1] != "TABLE") {
        throw std::runtime_error("Błędna składnia CREATE TABLE");
    }

    const auto& table_name = tokens[2];
    
    // Sprawdź czy tabela już istnieje
    auto tables = db->list_tables();
    if (std::find(tables.begin(), tables.end(), table_name) != tables.end()) {
        throw std::runtime_error("Tabela '" + table_name + "' już istnieje!");
    }

    size_t pos = 3;
    auto columns = parse_columns_definition(tokens, pos);

    auto table = std::make_unique<Table>(table_name);
    for (const auto& col : columns) {
        table->add_column(col);
    }

    db->save_table_schema(*table);
    std::cout << "Utworzono tabelę '" << table_name << "'\n";
}

void SQLParser::handle_insert(const std::vector<std::string>& tokens) {
    if (tokens.size() < 4 || tokens[1] != "INTO") {
        throw std::runtime_error("Błędna składnia INSERT INTO");
    }

    const auto& table_name = tokens[2];
    auto table = db->load_table(table_name);
    
    // Sprawdź czy liczba wartości zgadza się z liczbą kolumn
    const auto& columns = table->get_columns();
    size_t pos = 3;
    size_t values_count = tokens.size() - pos;
    
    if (values_count != columns.size()) {
        throw std::runtime_error("Nieprawidłowa liczba wartości. Oczekiwano " + 
                               std::to_string(columns.size()) + " wartości, otrzymano " + 
                               std::to_string(values_count));
    }

    // Tworzenie wiersza z wartościami
    Row row;
    for (size_t i = 0; i < columns.size(); i++) {
        row.data[columns[i].name] = tokens[pos + i];
    }

    table->insert_row(row);
    db->save_table_data(*table);
    std::cout << "Dodano wiersz do tabeli '" << table_name << "'\n";
}

void SQLParser::handle_select(const std::vector<std::string>& tokens) {
    if (tokens.size() < 4) {
        throw std::runtime_error("Błędna składnia SELECT");
    }

    size_t pos = 1;
    std::vector<std::string> columns;
    
    // Obsługa SELECT *
    if (tokens[pos] == "*") {
        pos++;
    } else {
        columns = parse_column_list(tokens, pos);
    }

    if (pos >= tokens.size() || tokens[pos] != "FROM") {
        throw std::runtime_error("Brak FROM w zapytaniu SELECT");
    }
    pos++;

    const auto& table_name = tokens[pos++];
    auto table = db->load_table(table_name);

    // Jeśli była gwiazdka, pobierz wszystkie kolumny z tabeli
    if (tokens[1] == "*") {
        columns.clear();
        for (const auto& col : table->get_columns()) {
            columns.push_back(col.name);
        }
    } else {
        // Sprawdź czy wszystkie żądane kolumny istnieją
        const auto& table_columns = table->get_columns();
        for (const auto& requested_col : columns) {
            bool found = false;
            for (const auto& table_col : table_columns) {
                if (table_col.name == requested_col) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                throw std::runtime_error("Kolumna '" + requested_col + "' nie istnieje w tabeli '" + table_name + "'");
            }
        }
    }

    std::optional<std::string> where_condition;
    if (pos < tokens.size() && tokens[pos] == "WHERE") {
        pos++;
        auto [condition, _] = parse_where_clause(tokens, pos);
        where_condition = condition;
    }

    auto results = table->select(columns, where_condition);
    
    if (results.empty()) {
        std::cout << "Brak wyników.\n";
        return;
    }

    // Znajdź maksymalną szerokość dla każdej kolumny
    std::unordered_map<std::string, size_t> col_widths;
    for (const auto& col : columns) {
        col_widths[col] = col.length();
    }

    for (const auto& row : results) {
        for (const auto& [col, val] : row.data) {
            if (col_widths.find(col) != col_widths.end()) {  // tylko dla wybranych kolumn
                col_widths[col] = std::max(col_widths[col], val.length());
            }
        }
    }

    // Wyświetl nagłówki
    for (const auto& col : columns) {
        std::cout << "| " << std::setw(col_widths[col]) << col << " ";
    }
    std::cout << "|\n";

    // Wyświetl separator
    for (const auto& col : columns) {
        std::cout << "+-" << std::string(col_widths[col], '-') << "-";
    }
    std::cout << "+\n";

    // Wyświetl dane
    for (const auto& row : results) {
        for (const auto& col : columns) {
            auto it = row.data.find(col);
            std::string val = (it != row.data.end()) ? it->second : "";
            std::cout << "| " << std::setw(col_widths[col]) << val << " ";
        }
        std::cout << "|\n";
    }
}

void SQLParser::handle_update(const std::vector<std::string>& tokens) {
    if (tokens.size() < 5) {
        throw std::runtime_error("Invalid UPDATE syntax");
    }

    const auto& table_name = tokens[1];
    auto table = db->load_table(table_name);

    if (tokens[2] != "SET") {
        throw std::runtime_error("Missing SET in UPDATE statement");
    }

    size_t pos = 3;
    const auto& column = tokens[pos++];
    if (pos >= tokens.size() || tokens[pos] != "=") {
        throw std::runtime_error("Missing = in UPDATE statement");
    }
    pos++;
    const auto& value = tokens[pos++];

    std::string where_condition;
    if (pos < tokens.size() && tokens[pos] == "WHERE") {
        pos++;
        auto [condition, _] = parse_where_clause(tokens, pos);
        where_condition = condition;
    }

    table->update(column, value, where_condition);
    db->save_table_data(*table);
}

void SQLParser::handle_delete(const std::vector<std::string>& tokens) {
    if (tokens.size() < 3 || tokens[1] != "FROM") {
        throw std::runtime_error("Invalid DELETE syntax");
    }

    const auto& table_name = tokens[2];
    auto table = db->load_table(table_name);

    size_t pos = 3;
    std::string where_condition;
    if (pos < tokens.size() && tokens[pos] == "WHERE") {
        pos++;
        auto [condition, _] = parse_where_clause(tokens, pos);
        where_condition = condition;
    }

    table->delete_rows(where_condition);
    db->save_table_data(*table);
}

void SQLParser::handle_drop_table(const std::vector<std::string>& tokens) {
    if (tokens.size() < 3 || tokens[1] != "TABLE") {
        throw std::runtime_error("Błędna składnia DROP TABLE");
    }

    const auto& table_name = tokens[2];
    
    // Sprawdź czy tabela istnieje przed usunięciem
    auto tables = db->list_tables();
    if (std::find(tables.begin(), tables.end(), table_name) == tables.end()) {
        throw std::runtime_error("Tabela '" + table_name + "' nie istnieje!");
    }

    db->delete_table(table_name);
    std::cout << "Usunięto tabelę '" << table_name << "'\n";
}

std::vector<Column> SQLParser::parse_columns_definition(const std::vector<std::string>& tokens, size_t& pos) {
    std::vector<Column> columns;
    
    while (pos < tokens.size()) {
        Column col;
        
        // Nazwa kolumny
        if (pos >= tokens.size()) break;
        col.name = tokens[pos++];
        
        // Typ kolumny
        if (pos >= tokens.size()) break;
        col.type = tokens[pos++];
        
        // Domyślne wartości
        col.is_primary_key = false;
        col.is_nullable = true;
        
        // Sprawdź constrainty
        while (pos < tokens.size() && tokens[pos] != ",") {
            if (tokens[pos] == "PRIMARY" && pos + 1 < tokens.size() && tokens[pos + 1] == "KEY") {
                col.is_primary_key = true;
                pos += 2;
            } else if (tokens[pos] == "NOT" && pos + 1 < tokens.size() && tokens[pos + 1] == "NULL") {
                col.is_nullable = false;
                pos += 2;
            } else {
                break;  // Nieznany constraint
            }
        }
        
        columns.push_back(col);
        
        // Jeśli następny token to przecinek, przejdź dalej
        if (pos < tokens.size() && tokens[pos] == ",") {
            pos++;
        }
    }
    
    // Sprawdź czy jest przynajmniej jedna kolumna
    if (columns.empty()) {
        throw std::runtime_error("Tabela musi mieć przynajmniej jedną kolumnę");
    }
    
    return columns;
}

std::vector<std::string> SQLParser::parse_column_list(const std::vector<std::string>& tokens, size_t& pos) {
    std::vector<std::string> columns;
    
    while (pos < tokens.size() && tokens[pos] != "FROM" && tokens[pos] != "WHERE") {
        if (tokens[pos] == ",") {
            pos++;
            continue;
        }
        columns.push_back(tokens[pos++]);
    }
    
    return columns;
}

std::pair<std::string, std::vector<std::string>> SQLParser::parse_where_clause(const std::vector<std::string>& tokens, size_t& pos) {
    std::string condition;
    std::vector<std::string> params;
    
    while (pos < tokens.size()) {
        condition += tokens[pos] + " ";
        pos++;
    }
    
    return {condition, params};
} 