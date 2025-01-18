cmake_minimum_required(VERSION 3.8)
project(CppDatabase)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dodaj pliki źródłowe
add_executable(${PROJECT_NAME} 
    CppDatabase.cpp
    class_definitions/Table.cpp
    class_definitions/DatabasePersistence.cpp
        class_definitions/SQLParser.cpp
)

# Dodaj ścieżki include
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
