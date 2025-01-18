#include "handlers/SqlCommandHandler.hpp"
#include <sstream>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <stdexcept>

std::vector<std::string> SqlCommandHandler::tokenize(const std::string &query)
{
    std::vector<std::string> tokens;
    std::istringstream iss(query);
    std::string token;

    while (iss >> token)
    {
        token.erase(std::ranges::remove(token, ',').begin(), token.end());
        token.erase(std::ranges::remove(token, '(').begin(), token.end());
        token.erase(std::ranges::remove(token, ')').begin(), token.end());

        if (!token.empty())
        {
            to_upper(token);
            tokens.push_back(token);
        }
    }
    
    return tokens;
}

void SqlCommandHandler::to_upper(std::string &str)
{
    std::ranges::transform(str, str.begin(), ::toupper);
}

auto SqlCommandHandler::exec_sql_command(const std::unique_ptr<InputBuffer>& input_buffer) -> SqlCommandResults
{
    const auto tokens = tokenize(input_buffer->get_buffer());
    if (tokens.empty())
    {
        return SqlCommandResults::EMPTY_QUERY;
    }

    if (const auto &command = tokens[0]; command == "CREATE")
    {
        return handle_create_table(tokens);
    }
    else if (command == "INSERT")
    {
        return handle_insert(tokens);
    }
    else if (command == "SELECT")
    {
        return handle_select(tokens);
    }
    else if (command == "UPDATE")
    {
        return handle_update(tokens);
    }
    else if (command == "DELETE")
    {
        return handle_delete(tokens);
    }
    else if (command == "DROP")
    {
        return handle_drop_table(tokens);
    }
    else
    {
        return SqlCommandResults::UNKNOWN_COMMAND;
    }
}

SqlCommandResults SqlCommandHandler::handle_create_table(const std::vector<std::string> &tokens)
{
    if (tokens.size() < 4 || tokens[1] != "TABLE")
    {
        return SqlCommandResults::INCORRECT_EXPRESSION;
    }

    const auto &table_name = tokens[2];

    if (auto tables = db->list_tables(); std::ranges::find(tables, table_name) != tables.end())
    {
        return SqlCommandResults::TABLE_ALREADY_EXISTS;
    }

    size_t pos = 3;
    auto columns = parse_columns_definition(tokens, 3);

    const auto table = std::make_unique<Table>(table_name);
    for (const auto &col : columns)
    {
        table->add_column(col);
    }

    db->save_table_schema(*table);
    std::cout << table_name << " created successfuly \n";
    return SqlCommandResults::SUCCESS;
}

SqlCommandResults SqlCommandHandler::handle_insert(const std::vector<std::string> &tokens) const {
    if (tokens.size() < 4 || tokens[1] != "INTO")
    {
        return SqlCommandResults::INCORRECT_EXPRESSION;
    }

    const auto &table_name = tokens[2];
    const auto table = db->load_table(table_name);

    const auto &columns = table->get_columns();
    constexpr auto pos = 3;

    if (const auto values_count = tokens.size() - pos; values_count != columns.size())
    {
        return SqlCommandResults::INCORRECT_EXPRESSION;
    }

    Row row;
    for (size_t i = 0; i < columns.size(); i++)
    {
        row.data[columns[i].name] = tokens[pos + i];
    }

    table->insert_row(row);
    db->save_table_data(*table);
    return SqlCommandResults::SUCCESS;
}

SqlCommandResults SqlCommandHandler::handle_select(const std::vector<std::string> &tokens)
{
    if (tokens.size() < 4)
    {
        return SqlCommandResults::INCORRECT_EXPRESSION;
    }

    auto pos = 1;
    std::vector<std::string> columns;

    if (tokens[1] == "*")
    {
        pos++;
    }
    else
    {
        columns = parse_column_list(tokens, pos);
    }

    if (pos >= tokens.size() || tokens[pos] != "FROM")
    {
        return SqlCommandResults::INCORRECT_EXPRESSION;
    }
    pos++;

    const auto &table_name = tokens[pos++];
    const auto table = db->load_table(table_name);

    if (tokens[1] == "*")
    {
        columns.clear();
        for (const auto &col : table->get_columns())
        {
            columns.push_back(col.name);
        }
    }
    else
    {
        const auto &table_columns = table->get_columns();
        for (const auto &requested_col : columns)
        {
            bool found = false;
            for (const auto &table_col : table_columns)
            {
                if (table_col.name == requested_col)
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                return SqlCommandResults::INCORRECT_EXPRESSION;
            }
        }
    }

    std::optional<std::string> where_condition;
    if (pos < tokens.size() && tokens[pos] == "WHERE")
    {
        pos++;
        auto [condition, _] = parse_where_clause(tokens, pos);
        where_condition = condition;
    }

    auto results = table->select(columns, where_condition);

    if (results.empty())
    {
        std::cout << "Brak wyników.\n";
        return SqlCommandResults::SUCCESS;
    }


    std::unordered_map<std::string, size_t> col_widths;
    for (const auto &col : columns)
    {
        col_widths[col] = col.length();
    }

    for (const auto &row : results)
    {
        for (const auto &[col, val] : row.data)
        {
            if (col_widths.contains(col))
            {
                col_widths[col] = std::max(col_widths[col], val.length());
            }
        }
    }

    for (const auto &col : columns)
    {
        std::cout << "| " << std::setw(col_widths[col]) << col << " ";
    }
    std::cout << "|\n";

    for (const auto &col : columns)
    {
        std::cout << "+-" << std::string(col_widths[col], '-') << "-";
    }
    std::cout << "+\n";

    for (const auto &[data] : results)
    {
        for (const auto &col : columns)
        {
            auto it = data.find(col);
            std::string val = (it != data.end()) ? it->second : "";
            std::cout << "| " << std::setw(col_widths[col]) << val << " ";
        }
        std::cout << "|\n";
    }
    return SqlCommandResults::SUCCESS;
}

SqlCommandResults SqlCommandHandler::handle_update(const std::vector<std::string> &tokens)
{
    if (tokens.size() < 5)
    {
        return SqlCommandResults::INCORRECT_EXPRESSION;
    }

    const auto &table_name = tokens[1];
    auto table = db->load_table(table_name);

    if (tokens[2] != "SET")
    {
        return SqlCommandResults::INCORRECT_EXPRESSION;
    }

    auto pos = 3;
    const auto &column = tokens[pos++];
    if (pos >= tokens.size() || tokens[pos] != "=")
    {
        return SqlCommandResults::INCORRECT_EXPRESSION;
    }
    pos++;
    const auto &value = tokens[pos++];

    std::string where_condition;
    if (pos < tokens.size() && tokens[pos] == "WHERE")
    {
        pos++;
        auto [condition, _] = parse_where_clause(tokens, pos);
        where_condition = condition;
    }

    table->update(column, value, where_condition);
    db->save_table_data(*table);
    return SqlCommandResults::SUCCESS;
}

SqlCommandResults SqlCommandHandler::handle_delete(const std::vector<std::string> &tokens)
{
    if (tokens.size() < 3 || tokens[1] != "FROM")
    {
        return SqlCommandResults::INCORRECT_EXPRESSION;
    }

    const auto &table_name = tokens[2];
    const auto table = db->load_table(table_name);

    std::string where_condition;
    if (auto pos = 3; pos < tokens.size() && tokens[pos] == "WHERE")
    {
        pos++;
        auto [condition, _] = parse_where_clause(tokens, pos);
        where_condition = condition;
    }

    table->delete_rows(where_condition);
    db->save_table_data(*table);
    return SqlCommandResults::SUCCESS;
}

SqlCommandResults SqlCommandHandler::handle_drop_table(const std::vector<std::string> &tokens) const {
    if (tokens.size() < 3 || tokens[1] != "TABLE")
    {
        return SqlCommandResults::INCORRECT_EXPRESSION;
    }

    const auto &table_name = tokens[2];

    // Sprawdź czy tabela istnieje przed usunięciem
    auto tables = db->list_tables();
    if (std::find(tables.begin(), tables.end(), table_name) == tables.end())
    {
        return SqlCommandResults::TABLE_DOES_NOT_EXIST;
    }

    db->delete_table(table_name);
    std::cout << "Usunięto tabelę '" << table_name << "'\n";
    return SqlCommandResults::SUCCESS;
}

std::vector<Column> SqlCommandHandler::parse_columns_definition(const std::vector<std::string> &tokens, int position)
{
    std::vector<Column> columns;

    while (position < tokens.size())
    {
        Column col;

        if (position >= tokens.size())
            break;
        col.name = tokens[position++];

        if (position >= tokens.size())
            break;
        col.type = DatabasePersistence::string_to_column_type(tokens[position++]);

        col.is_primary_key = false;
        col.is_nullable = true;

        while (position < tokens.size() && tokens[position] != ",")
        {
            if (tokens[position] == "PRIMARY" && position + 1 < tokens.size() && tokens[position + 1] == "KEY")
            {
                col.is_primary_key = true;
                position += 2;
            }
            else if (tokens[position] == "NOT" && position + 1 < tokens.size() && tokens[position + 1] == "NULL")
            {
                col.is_nullable = false;
                position += 2;
            }
            else
            {
                break;
            }
        }

        columns.push_back(col);

        if (position < tokens.size() && tokens[position] == ",")
        {
            position++;
        }
    }

    if (columns.empty())
    {
        throw std::runtime_error("At least one column is expected");
    }

    return columns;
}

std::vector<std::string> SqlCommandHandler::parse_column_list(const std::vector<std::string> &tokens, int position)
{
    std::vector<std::string> columns;

    while (position < tokens.size() && tokens[position] != "FROM" && tokens[position] != "WHERE")
    {
        if (tokens[position] == ",")
        {
            position++;
            continue;
        }
        columns.push_back(tokens[position++]);
    }

    return columns;
}

std::pair<std::string, std::vector<std::string>> SqlCommandHandler::parse_where_clause(const std::vector<std::string> &tokens, int position)
{
    std::string condition;
    std::vector<std::string> params;

    while (position < tokens.size())
    {
        condition += tokens[position] + " ";
        position++;
    }

    return {condition, params};
}