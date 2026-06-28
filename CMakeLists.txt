project(bincache)

set(CMAKE_CXX_FLAGS -Wall -Wextra --pendatic)
set(CMAKE_CXX_STANDARD 26)

add_executable(${PROJECT_NAME}
    src/main.cpp
)
