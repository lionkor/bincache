add_subdirectory(abseil-cpp)

add_library(spdlog STATIC
    spdlog/src/spdlog.cpp
    spdlog/src/async.cpp
    spdlog/src/bundled_fmtlib_format.cpp
    spdlog/src/cfg.cpp
    spdlog/src/color_sinks.cpp
    spdlog/src/file_sinks.cpp
    spdlog/src/stdout_sinks.cpp
)
target_include_directories(spdlog PUBLIC spdlog/include)
target_compile_definitions(spdlog PUBLIC SPDLOG_COMPILED_LIB)

#add_library(arena_allocator STATIC
#    arena-allocator/arena.c
#)
#target_include_directories(spdlog PUBLIC arena-allocator/)
#target_compile_definitions(arena_allocator PUBLIC ARENA_IMPLEMENTATION)
