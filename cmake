cmake_minimum_required(VERSION 3.16)
project(ChatServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network Sql)

file(GLOB_RECURSE SRC_FILES sources/*.cpp)
file(GLOB_RECURSE HEADER_FILES sources/*.h)

add_executable(ChatServer ${SRC_FILES} ${HEADER_FILES})

target_link_libraries(ChatServer PRIVATE
    Qt6::Core Qt6::Widgets Qt6::Network Qt6::Sql
)
