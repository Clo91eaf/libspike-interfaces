cmake_minimum_required(VERSION 3.20)
project(spike-interfaces-test)

set(CMAKE_CXX_STANDARD 17)

add_executable(spike-interfaces-test spike-interfaces-test.cc)

target_link_libraries(spike-interfaces-test PUBLIC spike-interfaces)

install(TARGETS spike-interfaces-test)