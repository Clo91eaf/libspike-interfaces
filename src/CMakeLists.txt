cmake_minimum_required(VERSION 3.20)
project(test)

set(CMAKE_CXX_STANDARD 17)

find_package(libspike-interfaces REQUIRED)

add_executable(test test.cc)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(test PUBLIC libspike-interfaces)

install(TARGETS test)