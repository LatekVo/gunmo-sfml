cmake_minimum_required(VERSION 3.22)
project(gunmo)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

find_package(SFML REQUIRED COMPONENTS graphics system window)
include_directories(${SFML_INCLUDE_DIR} rsc)

add_executable(gunmo
        main.cpp)

# add libs you need
set(SFML_LIBRARIES sfml-system sfml-graphics sfml-window)

# linking
target_link_libraries(gunmo ${SFML_LIBRARIES})