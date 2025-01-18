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

    auto save_table_schema(const Table& table) -> void;
    auto save_table_data(const Table& table) -> void;
    std::unique_ptr<Table> load_table(const std::string& table_name);
    auto delete_table(const std::string& table_name) -> void;
    std::vector<std::string> list_tables() const;

private:
    auto get_schema_path(const std::string& table_name) const -> std::string;
    auto get_data_path(const std::string& table_name) const -> std::string;
}; 