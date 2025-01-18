#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <optional>

struct Column {
    std::string name;
    std::string type;
    bool is_primary_key = false;
    bool is_nullable = true;
};

struct Row {
    std::unordered_map<std::string, std::string> data;
};

class Table {
    std::string name;
    std::vector<Column> columns;
    std::vector<Row> rows;
    std::string primary_key_column;

public:
    explicit Table(std::string table_name) : name(std::move(table_name)) {}

    void add_column(const Column& column);
    void set_primary_key(const std::string& column_name);
    void add_foreign_key(const std::string& column_name, 
                        const std::string& foreign_table, 
                        const std::string& foreign_column);

    void insert_row(const Row& row);
    std::vector<Row> select(const std::vector<std::string>& columns,
                           const std::optional<std::string>& where_condition = std::nullopt);
    void update(const std::string& column, 
                const std::string& value,
                const std::string& where_condition);
    void delete_rows(const std::string& where_condition);
    // Gettery
    [[nodiscard]] const std::string& get_name() const { return name; }
    [[nodiscard]] const std::vector<Column>& get_columns() const { return columns; }
    [[nodiscard]] const std::vector<Row>& get_rows() const { return rows; }
    [[nodiscard]] const std::string& get_primary_key_column() const { return primary_key_column; }
}; 