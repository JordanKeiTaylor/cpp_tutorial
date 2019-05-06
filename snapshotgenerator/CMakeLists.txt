cmake_minimum_required(VERSION 3.12)
project(Othello)

set(CMAKE_CXX_STANDARD 14)

set(COMMON_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

find_program(SPATIAL_CMD spatial)
configure_file(spatialos_worker_packages.json.in spatialos_worker_packages.json)
add_custom_target(dependencies ALL ${SPATIAL_CMD} package unpack --package_file ${CMAKE_CURRENT_BINARY_DIR}/spatialos_worker_packages.json
                  SOURCES ${CMAKE_CURRENT_BINARY_DIR}/spatialos_worker_packages.json)
link_directories(${CMAKE_CURRENT_BINARY_DIR}/dependencies/worker_sdk/lib)

add_subdirectory(schema)
add_subdirectory(snapshots)
