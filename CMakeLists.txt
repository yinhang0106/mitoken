cmake_minimum_required(VERSION 3.22)

project(mitoken LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# add third-party libraries
add_subdirectory(include/mitoken/extern/uftcpp)

add_library(mitoken INTERFACE)
target_include_directories(mitoken INTERFACE include)
target_link_libraries(mitoken INTERFACE utf8cpp)

add_subdirectory(tests)
