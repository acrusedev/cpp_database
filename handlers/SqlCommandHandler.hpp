#pragma once

#include <string>
#include <vector>
#include <memory>
#include "../class_definitions/DatabasePersistence.hpp"
#include "../class_definitions/InputBuffer.hpp"
#include "../types/enums.hpp"

class SqlCommandHandler {
    std::shared_ptr<DatabasePersistence> db;

    static std::vector<std::string> tokenize(const std::string& query);
    static void to_upper(std::string& str);

    SqlCommandResults handle_create_table(const std::vector<std::string>& tokens);
    SqlCommandResults handle_insert(const std::vector<std::string>& tokens) const;
    SqlCommandResults handle_select(const std::vector<std::string>& tokens);
    SqlCommandResults handle_update(const std::vector<std::string>& tokens);
    SqlCommandResults handle_delete(const std::vector<std::string>& tokens);
    SqlCommandResults handle_drop_table(const std::vector<std::string>& tokens) const;

    static std::vector<Column> parse_columns_definition(const std::vector<std::string>& tokens, int position);
    static std::vector<std::string> parse_column_list(const std::vector<std::string>& tokens, int position);
    std::pair<std::string, std::vector<std::string>> parse_where_clause(const std::vector<std::string>& tokens, int position);

public:
    explicit SqlCommandHandler(std::shared_ptr<DatabasePersistence> database) : db(std::move(database)) {}
    auto exec_sql_command(const std::unique_ptr<InputBuffer>& input_buffer) -> SqlCommandResults;
}; 