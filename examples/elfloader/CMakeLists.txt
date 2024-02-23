cmake_minimum_required(VERSION 3.20)
project(elfloader)
set(CMAKE_CXX_STANDARD 17)

find_package(libspike_interfaces REQUIRED)
find_package(libspike REQUIRED)

add_executable(elfloader elfloader.c)

target_link_libraries(elfloader PUBLIC libspike_interfaces libspike)
target_include_directories(elfloader PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS elfloader RUNTIME)