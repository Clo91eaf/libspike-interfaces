cmake_minimum_required(VERSION 3.20)
project(libspike-interfaces)
set(CMAKE_CXX_STANDARD 17)

message(STATUS "Project '${PROJECT_NAME}' build type: ${CMAKE_BUILD_TYPE}")

find_package(libspike REQUIRED)

add_library(spike-interfaces SHARED spike-interfaces.cc)

target_link_libraries(spike-interfaces PUBLIC libspike)
target_include_directories(spike-interfaces PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS spike-interfaces)