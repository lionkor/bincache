cmake_minimum_required(VERSION 4.3)
set(CMAKE_CXX_COMPILER clang++)

project(bincache LANGUAGES CXX C)

include(CTest)

set(CMAKE_CXX_FLAGS_DEBUG "-O0 -ggdb")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 26)

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -flto -march=native -ggdb")

add_subdirectory(vendor)

add_executable(${PROJECT_NAME}
    src/cache.cpp
    src/cache.hpp
    src/defer.hpp
    src/main.cpp
)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra --pedantic)
target_include_directories(${PROJECT_NAME} PRIVATE vendor/asio/include)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog absl::flat_hash_map)

# --- tests ---

add_executable(${PROJECT_NAME}_tests
    src/cache.cpp
    src/cache.test.cpp
)
target_compile_options(${PROJECT_NAME}_tests PRIVATE -Wall -Wextra --pedantic)
target_include_directories(${PROJECT_NAME}_tests PRIVATE vendor/asio/include)
target_link_libraries(${PROJECT_NAME}_tests PRIVATE spdlog absl::flat_hash_map GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME}_tests)
