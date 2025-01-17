#pragma once

#include <string>
#include <vector>
#include <memory>
#include "DatabasePersistence.hpp"

class SQLParser {
private:
    std::shared_ptr<DatabasePersistence> db;

    // Pomocnicze metody do parsowania
    std::vector<std::string> tokenize(const std::string& query);
    void to_upper(std::string& str);
    
    // Metody do obsługi poszczególnych typów zapytań
    void handle_create_table(const std::vector<std::string>& tokens);
    void handle_insert(const std::vector<std::string>& tokens);
    void handle_select(const std::vector<std::string>& tokens);
    void handle_update(const std::vector<std::string>& tokens);
    void handle_delete(const std::vector<std::string>& tokens);
    void handle_drop_table(const std::vector<std::string>& tokens);

    // Parsowanie części zapytań
    std::vector<Column> parse_columns_definition(const std::vector<std::string>& tokens, size_t& pos);
    std::vector<std::string> parse_column_list(const std::vector<std::string>& tokens, size_t& pos);
    std::pair<std::string, std::vector<std::string>> parse_where_clause(const std::vector<std::string>& tokens, size_t& pos);

public:
    explicit SQLParser(std::shared_ptr<DatabasePersistence> database) : db(std::move(database)) {}
    
    // Główna metoda do wykonywania zapytań SQL
    void execute_query(const std::string& query);
}; 