#include "DatabasePersistence.hpp"

#include <algorithm>
#include <fstream>
#include <sstream>

auto DatabasePersistence::save_table_schema(const Table& table) const -> void {
    std::ofstream file(get_schema_path(table.get_name()));
    if (!file.is_open()) {
        throw std::runtime_error("Unknown error while creating a schema for table : " + table.get_name());
    }


    //Table schema:
    //              Table name
    //              Number of columns
    //              column data type: COL_NAME | COL_DATA_TYPE | COL_IS_PRIMARY_KEY  (0 || 1) | COL_IS_NULLABLE (0 || 1) 
    
    file << table.get_name() << "\n";
    
    auto columns = table.get_columns();
    file << columns.size() << "\n";
    
    for (const auto&[name, type, is_primary_key, is_nullable] : columns) {
        file << name << "|"
             << column_type_to_string(type) << "|"
             << (is_primary_key ? "1" : "0") << "|"
             << (is_nullable ? "1" : "0") << "\n";
    }
}
static auto to_upper(std::string& str) -> std::string {
    std::ranges::transform(str, str.begin(), ::toupper);
}

auto DatabasePersistence::string_to_column_type(const std::string& column_type) -> ColumnType {
    if (column_type == "INTEGER") return ColumnType::INTEGER;
    if (column_type == "BOOLEAN") return ColumnType::BOOLEAN;
    if (column_type == "TEXT") return ColumnType::TEXT;
    throw std::runtime_error("Unknown column type: " + column_type);
}

auto DatabasePersistence::column_type_to_string(const ColumnType& type) -> std::string {
    switch (type) {
        case ColumnType::INTEGER:
            return "INTEGER";
        case ColumnType::BOOLEAN:
            return "BOOLEAN";
        case ColumnType::TEXT:
            return "TEXT";
        default:
            return "TEXT";
    }
}



auto DatabasePersistence::save_table_data(const Table& table) const -> void {
    std::ofstream file(get_data_path(table.get_name()));

    //Data schema:
    //COL_NAME=VALUE|COL_NAME=VALUE...

    for (const auto&[data] : table.get_rows()) {
        bool first = true;
        for (const auto& [col, value] : data) {
            if (!first) file << "|";
            file << col << "=" << value;
            first = false;
        }
        file << "\n";
    }
}

auto DatabasePersistence::load_table(const std::string& table_name) const -> std::unique_ptr<Table> {
    std::ifstream schema_file(get_schema_path(table_name));
    if (!schema_file.is_open()) {
        throw std::runtime_error("Table does not exist!: " + table_name);
    }

    std::string name;
    std::getline(schema_file, name);
    auto table = std::make_unique<Table>(name);

    std::string col_count_str;
    std::getline(schema_file, col_count_str);
    auto col_count = std::stoul(col_count_str);

    for (auto i = 0; i < col_count; i++) {
        std::string line;
        if (!std::getline(schema_file, line)) {
           break;
        }

        std::stringstream string_stream(line);
        Column column;

        if (!std::getline(string_stream, column.name, '|')) break;

        std::string type_str;
        if (!std::getline(string_stream, type_str, '|')) break;
        column.type = string_to_column_type(type_str);

        std::string is_primary_key;
        if (!std::getline(string_stream, is_primary_key, '|')) break;
        column.is_primary_key = (is_primary_key == "1");

        std::string is_null;
        if (!std::getline(string_stream, is_null, '|')) break;
        column.is_nullable = (is_null == "1");
        
        table->add_column(column);
    }

    if (std::ifstream data_file(get_data_path(table_name)); data_file.is_open()) {
        std::string line;
        while (std::getline(data_file, line)) {
            Row row;
            std::stringstream string_stream(line);
            std::string pair;
            
            while (std::getline(string_stream, pair, '|')) {
                if (auto pos = pair.find('='); pos != std::string::npos) {
                    std::string col = pair.substr(0, pos);
                    std::string val = pair.substr(pos + 1);
                    row.data[col] = val;
                }
            }
            
            table->insert_row(row);
        }
    }

    return table;
}

auto DatabasePersistence::delete_table(const std::string& table_name) const -> void {
    std::filesystem::remove(get_schema_path(table_name));
    std::filesystem::remove(get_data_path(table_name));
}

auto DatabasePersistence::list_tables() const -> std::vector<std::string> {
    std::vector<std::string> tables;
    for (const auto& entry : std::filesystem::directory_iterator(db_directory)) {
        if (entry.path().extension() == SCHEMA_EXTENSION) {
            tables.push_back(entry.path().stem().string());
        }
    }
    return tables;
}

auto DatabasePersistence::get_schema_path(const std::string& table_name) const -> std::string {
    return (std::filesystem::path(db_directory) / (table_name + SCHEMA_EXTENSION)).string();
}

auto DatabasePersistence::get_data_path(const std::string& table_name) const -> std::string{
    return (std::filesystem::path(db_directory) / (table_name + DATA_EXTENSION)).string();
}