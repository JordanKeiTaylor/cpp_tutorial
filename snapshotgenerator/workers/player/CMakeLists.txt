# Builds a SpatialOS worker using the C++ API
project(player)
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

# The worker binary.
# Make sure you add all worker and generated sources to the executable
file(GLOB_RECURSE SOURCE_FILES
        "src/*.cc"
        "src/*.cpp"
        "src/*.h"
        "src/*.hpp"
        )

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

set(VS_STARTUP_PROJECT ${CMAKE_PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} Schema WorkerSdk CoreSdk libprotobuf RakNetLibStatic grpc++ grpc gpr ssl zlibstatic)
target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE
        ${GENERATED_CODE_DIR}
        "${WORKER_SDK_DIR}/include")

# Set artifact subdirectories.
# WORKER_BUILD_DIR should not be changed so that spatial local launch
# and spatial upload can find the worker assemblies
set(WORKER_BUILD_DIR "${CMAKE_SOURCE_DIR}/build/assembly/worker")

# The worker in a zip.
add_custom_target(
        ${PROJECT_NAME}Zip ALL
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory "${WORKER_BUILD_DIR}"
        COMMAND ${SPATIAL_CMD} file zip -b "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
        -o "${WORKER_BUILD_DIR}/${PROJECT_NAME}"
        --worker_platform=current
        "$<TARGET_FILE_NAME:${PROJECT_NAME}>"
        DEPENDS ${PROJECT_NAME})