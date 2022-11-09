cmake_minimum_required(VERSION 3.16)

project(MemPool)

set(CMAKE_CXX_STANDARD 17)

# 头文件
include_directories(
        ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/src/ThreadMemPool
        ${PROJECT_SOURCE_DIR}/src/CentralMemPool
        ${PROJECT_SOURCE_DIR}/test
)

# 源文件
file(GLOB_RECURSE MemPool
        ${PROJECT_SOURCE_DIR}/src/ThreadMemPool/*.cpp
        ${PROJECT_SOURCE_DIR}/src/CentralMemPool/*.cpp
        ${PROJECT_SOURCE_DIR}/test/*.cpp
        )

# 执行文件
add_executable(MemPool ${MemPool})

find_package(Threads REQUIRED)
target_link_libraries(MemPool Threads::Threads)