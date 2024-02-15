cmake_minimum_required(VERSION 3.20)
project(test)
set(CMAKE_CXX_STANDARD 17)

find_package(libspike REQUIRED)

add_executable(test test.cc spike-interfaces.cc)

target_link_libraries(test PUBLIC libspike)
target_include_directories(test PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS test)