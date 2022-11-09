cmake_minimum_required(VERSION 3.16)

project(MemPool)

set(CMAKE_CXX_STANDARD 11)

# 头文件
include_directories(
        ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/src/MemPool
        ${PROJECT_SOURCE_DIR}/test
)

# 源文件
file(GLOB_RECURSE MemPool
        ${PROJECT_SOURCE_DIR}/src/MemPool/*.cpp
        ${PROJECT_SOURCE_DIR}/test/*.cpp
        )

# 执行文件
add_executable(MemPool ${MemPool})

find_package(Threads REQUIRED)
target_link_libraries(MemPool Threads::Threads)