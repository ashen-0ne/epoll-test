cmake_minimum_required(VERSION 3.0)
project (epoll_test)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/cmake_build)

include_directories(
    ${PROJECT_SOURCE_DIR}/inc
)

file(GLOB_RECURSE API_SRC_LIST 
 "test.h" "test.cpp" "main.cpp"
)

add_executable(${PROJECT_NAME} ${API_SRC_LIST})
target_link_libraries(${PROJECT_NAME} pthread rt)