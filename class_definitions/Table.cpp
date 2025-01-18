#include "Table.hpp"
#include <stdexcept>
#include <algorithm>



void Table::add_column(const Column& column) {
    const auto it = std::ranges::find_if(columns,
                                         [&column](const Column& existing) {
                                             return existing.name == column.name;
                                         });

    if (it != columns.end()) {
        throw std::runtime_error("Column already exists: " + column.name);
    }

    if (column.is_primary_key) {
        const auto pk = std::ranges::find_if(columns,
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
    auto it = std::ranges::find_if(columns,
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

auto Table::string_to_column_type(const std::string& type_str) -> ColumnType {
    if (type_str == "INTEGER") return ColumnType::INTEGER;
    if (type_str == "BOOLEAN") return ColumnType::BOOLEAN;
    if (type_str == "TEXT") return ColumnType::TEXT;
    throw std::runtime_error("Unknown column type: " + type_str);
}

auto Table::column_type_to_string(ColumnType type) -> std::string {
    switch (type) {
        case ColumnType::INTEGER: return "INTEGER";
        case ColumnType::BOOLEAN: return "BOOLEAN";
        case ColumnType::TEXT: return "TEXT";
        default: return "TEXT";
    }
}

std::vector<Row> Table::select_where(const std::vector<std::string>& columns, const WhereClause& where) {
    std::vector<Row> result;
    
    for (const auto& row : rows) {
        bool matches = true;
        
        for (const auto& condition : where.conditions) {
            auto it = row.data.find(condition.column);
            if (it == row.data.end()) {
                matches = false;
                break;
            }
            
            const std::string& row_value = it->second;
            
            switch (condition.op) {
                case WhereOperator::EQUALS:
                    matches = (row_value == condition.value);
                    break;
                case WhereOperator::GREATER:
                    matches = (std::stoi(row_value) > std::stoi(condition.value));
                    break;
                case WhereOperator::LESS:
                    matches = (std::stoi(row_value) < std::stoi(condition.value));
                    break;
                case WhereOperator::GREATER_EQ:
                    matches = (std::stoi(row_value) >= std::stoi(condition.value));
                    break;
                case WhereOperator::LESS_EQ:
                    matches = (std::stoi(row_value) <= std::stoi(condition.value));
                    break;
            }
            
            if (!matches) break;
        }
        
        if (matches) {
            if (columns.empty()) {
                result.push_back(row);
            } else {
                Row filtered_row;
                for (const auto& col : columns) {
                    auto it = row.data.find(col);
                    if (it != row.data.end()) {
                        filtered_row.data[col] = it->second;
                    }
                }
                result.push_back(filtered_row);
            }
        }
    }
    
    return result;
}