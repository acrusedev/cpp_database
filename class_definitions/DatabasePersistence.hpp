#pragma once

#include <string>
#include <filesystem>
#include "class_definitions/Table.hpp"

class DatabasePersistence {
private:
    static constexpr const char* SCHEMA_EXTENSION = ".schema";
    static constexpr const char* DATA_EXTENSION = ".data";
    std::string db_directory;

public:
    explicit DatabasePersistence(std::string directory) 
        : db_directory(std::move(directory)) {
        std::filesystem::create_directories(db_directory);
    }

    void save_table_schema(const Table& table);
    void save_table_data(const Table& table);
    std::unique_ptr<Table> load_table(const std::string& table_name);
    void delete_table(const std::string& table_name);
    std::vector<std::string> list_tables() const;

private:
    std::string get_schema_path(const std::string& table_name) const;
    std::string get_data_path(const std::string& table_name) const;
}; 