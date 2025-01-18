#pragma once

#include <string>
#include <filesystem>
#include "class_definitions/Table.hpp"

class DatabasePersistence {
private:
    static constexpr auto SCHEMA_EXTENSION = ".schema";
    static constexpr auto DATA_EXTENSION = ".data";
    std::string db_directory;

public:
    explicit DatabasePersistence(std::string directory) 
        : db_directory(std::move(directory)) {
        std::filesystem::create_directories(db_directory);
    }

    auto save_table_schema(const Table& table) const -> void;
    auto save_table_data(const Table& table) const -> void;
    auto delete_table(const std::string& table_name) const -> void;
    [[nodiscard]] auto load_table(const std::string& table_name) const -> std::unique_ptr<Table>;
    [[nodiscard]] auto list_tables() const -> std::vector<std::string>;

private:
    [[nodiscard]] auto get_schema_path(const std::string& table_name) const -> std::string;
    [[nodiscard]] auto get_data_path(const std::string& table_name) const -> std::string;
}; 