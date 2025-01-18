#include "Table.hpp"
#include <stdexcept>
#include <algorithm>

void Table::add_column(const Column& column) {
    auto it = std::find_if(columns.begin(), columns.end(),
        [&column](const Column& existing) {
            return existing.name == column.name;
        });
    
    if (it != columns.end()) {
        throw std::runtime_error("Column already exists: " + column.name);
    }

    if (column.is_primary_key) {
        auto pk = std::find_if(columns.begin(), columns.end(),
            [](const Column& existing) {
                return existing.is_primary_key;
            });
        
        if (pk != columns.end()) {
            throw std::runtime_error("Table already has a primary key: " + pk->name);
        }
        primary_key_column = column.name;
    }

    columns.push_back(column);
}

void Table::set_primary_key(const std::string& column_name) {
    auto it = std::find_if(columns.begin(), columns.end(),
        [&column_name](const Column& column) {
            return column.name == column_name;
        });
    
    if (it == columns.end()) {
        throw std::runtime_error("Column not found: " + column_name);
    }

    for (auto& column : columns) {
        if (column.is_primary_key) {
            column.is_primary_key = false;
        }
    }

    it->is_primary_key = true;
    primary_key_column = column_name;
}

void Table::add_foreign_key(const std::string& column_name, 
                          const std::string& foreign_table, 
                          const std::string& foreign_column) {
    auto it = std::find_if(columns.begin(), columns.end(),
        [&column_name](const Column& column) {
            return column.name == column_name;
        });
    
    if (it == columns.end()) {
        throw std::runtime_error("Column not found: " + column_name);
    }

}

void Table::insert_row(const Row& row) {
    for (const auto& column : columns) {
        if (!column.is_nullable && row.data.find(column.name) == row.data.end()) {
            throw std::runtime_error("Missing required column in row: " + column.name);
        }
    }

    if (!primary_key_column.empty()) {
        auto pk_value = row.data.find(primary_key_column);
        if (pk_value == row.data.end()) {
            throw std::runtime_error("Missing primary key value");
        }

        for (const auto& existing_row : rows) {
            if (existing_row.data.at(primary_key_column) == pk_value->second) {
                throw std::runtime_error("Duplicate primary key value: " + pk_value->second);
            }
        }
    }

    rows.push_back(row);
}

std::vector<Row> Table::select(const std::vector<std::string>& select_columns,
                             const std::optional<std::string>& where_condition) {
    // TODO: Implement WHERE condition parsing
    std::vector<Row> result;
    
    if (select_columns.empty()) {
        return rows;
    }

    for (const auto& row : rows) {
        Row filtered_row;
        for (const auto& col : select_columns) {
            auto it = row.data.find(col);
            if (it != row.data.end()) {
                filtered_row.data[col] = it->second;
            }
        }
        result.push_back(filtered_row);
    }

    return result;
}

void Table::update(const std::string& column, 
                  const std::string& value,
                  const std::string& where_condition) {
    // TODO: Implement WHERE condition parsing
    auto it = std::find_if(columns.begin(), columns.end(),
        [&column](const Column& col) {
            return col.name == column;
        });
    
    if (it == columns.end()) {
        throw std::runtime_error("Column not found: " + column);
    }

    for (auto& row : rows) {
        row.data[column] = value;
    }
}

void Table::delete_rows(const std::string& where_condition) {
    // TODO: Implement WHERE condition parsing
    rows.clear();
} 