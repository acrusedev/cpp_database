#include "DatabasePersistence.hpp"
#include <fstream>
#include <sstream>

void DatabasePersistence::save_table_schema(const Table& table) {
    std::ofstream file(get_schema_path(table.get_name()));
    if (!file.is_open()) {
        throw std::runtime_error("Nie można utworzyć pliku schematu dla tabeli: " + table.get_name());
    }
    
    // Zapisz nazwę tabeli
    file << table.get_name() << "\n";
    
    // Zapisz liczbę kolumn
    const auto& columns = table.get_columns();
    file << columns.size() << "\n";
    
    // Zapisz kolumny
    for (const auto& col : columns) {
        file << col.name << "|" 
             << col.type << "|"
             << (col.is_primary_key ? "1" : "0") << "|"
             << (col.is_nullable ? "1" : "0");

        file << "\n";
    }
}

void DatabasePersistence::save_table_data(const Table& table) {
    std::ofstream file(get_data_path(table.get_name()));
    
    // Zapisz dane
    for (const auto& row : table.get_rows()) {
        bool first = true;
        for (const auto& [col, value] : row.data) {
            if (!first) file << "|";
            file << col << "=" << value;
            first = false;
        }
        file << "\n";
    }
}

std::unique_ptr<Table> DatabasePersistence::load_table(const std::string& table_name) {
    std::ifstream schema_file(get_schema_path(table_name));
    if (!schema_file.is_open()) {
        throw std::runtime_error("Tabela nie istnieje: " + table_name);
    }

    // Wczytaj nazwę tabeli
    std::string name;
    std::getline(schema_file, name);
    auto table = std::make_unique<Table>(name);
    
    // Wczytaj liczbę kolumn
    std::string col_count_str;
    std::getline(schema_file, col_count_str);
    size_t col_count = std::stoul(col_count_str);
    
    // Wczytaj kolumny
    for (size_t i = 0; i < col_count; i++) {
        std::string line;
        if (!std::getline(schema_file, line)) {
            throw std::runtime_error("Błąd podczas wczytywania schematu tabeli: niepełne dane");
        }

        std::stringstream ss(line);
        std::string item;
        Column column;
        
        // Nazwa kolumny
        if (!std::getline(ss, column.name, '|')) break;
        
        // Typ kolumny
        if (!std::getline(ss, column.type, '|')) break;
        
        // Primary key flag
        std::string is_pk;
        if (!std::getline(ss, is_pk, '|')) break;
        column.is_primary_key = (is_pk == "1");
        
        // Nullable flag
        std::string is_null;
        if (!std::getline(ss, is_null, '|')) break;
        column.is_nullable = (is_null == "1");
        
        table->add_column(column);
    }

    // Wczytaj dane
    std::ifstream data_file(get_data_path(table_name));
    if (data_file.is_open()) {
        std::string line;
        while (std::getline(data_file, line)) {
            Row row;
            std::stringstream ss(line);
            std::string pair;
            
            while (std::getline(ss, pair, '|')) {
                size_t pos = pair.find('=');
                if (pos != std::string::npos) {
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

void DatabasePersistence::delete_table(const std::string& table_name) {
    std::filesystem::remove(get_schema_path(table_name));
    std::filesystem::remove(get_data_path(table_name));
}

std::vector<std::string> DatabasePersistence::list_tables() const {
    std::vector<std::string> tables;
    for (const auto& entry : std::filesystem::directory_iterator(db_directory)) {
        if (entry.path().extension() == SCHEMA_EXTENSION) {
            tables.push_back(entry.path().stem().string());
        }
    }
    return tables;
}

std::string DatabasePersistence::get_schema_path(const std::string& table_name) const {
    return (std::filesystem::path(db_directory) / (table_name + SCHEMA_EXTENSION)).string();
}

std::string DatabasePersistence::get_data_path(const std::string& table_name) const {
    return (std::filesystem::path(db_directory) / (table_name + DATA_EXTENSION)).string();
}