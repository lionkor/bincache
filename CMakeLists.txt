cmake_minimum_required(VERSION 4.3)
set(CMAKE_CXX_COMPILER clang++)

project(bincache LANGUAGES CXX)

set(CMAKE_CXX_FLAGS_DEBUG "-Wall -Wextra --pedantic -O0 -g0")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 26)

add_subdirectory(vendor)

add_executable(${PROJECT_NAME}
    src/main.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE vendor/asio/include)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog)
