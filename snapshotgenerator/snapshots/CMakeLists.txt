# Builds a SpatialOS worker using the C++ API
project(create_snapshot)
cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 14)
if(MSVC)
    add_definitions(/W2 /bigobj)
else()
    # Strict warnings.
    add_definitions(-Wall -Wextra -Werror -pedantic)
endif()

set(WORKER_SDK_DIR "${COMMON_BINARY_DIR}/dependencies/worker_sdk")
set(GENERATED_CODE_DIR "${COMMON_BINARY_DIR}/generated_src")

add_executable(${PROJECT_NAME} main.cpp)

set(VS_STARTUP_PROJECT ${CMAKE_PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} Schema WorkerSdk CoreSdk protobuf RakNetLibStatic grpc++ grpc gpr ssl z)
target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE
        ${GENERATED_CODE_DIR}
        "${WORKER_SDK_DIR}/include")

# The worker in a zip.
add_custom_target(create_default_snapshot ALL
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/default.snapshot
        DEPENDS ${PROJECT_NAME}
        BYPRODUCTS ${CMAKE_CURRENT_SOURCE_DIR}/default.snapshot)