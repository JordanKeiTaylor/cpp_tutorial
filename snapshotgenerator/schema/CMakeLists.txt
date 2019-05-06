project(Schema LANGUAGES CXX)

file(GLOB SCHEMA_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.schema)
string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "${COMMON_BINARY_DIR}/generated_src" SCHEMA_GEN_SRC ${SCHEMA_SRC})
string(REPLACE ".schema" ".h" SCHEMA_GEN_HDR ${SCHEMA_GEN_SRC})
string(REPLACE ".schema" ".cc" SCHEMA_GEN_SRC ${SCHEMA_GEN_SRC})

add_custom_target(schema-gen ${SPATIAL_CMD} schema generate --language=cpp
        --output ${COMMON_BINARY_DIR}/generated_src
        BYPRODUCTS ${SCHEMA_GEN_HDR} ${SCHEMA_GEN_SRC}
        ${COMMON_BINARY_DIR}/generated_src/improbable/standard_library.h
        ${COMMON_BINARY_DIR}/generated_src/improbable/standard_library.cc
        ${COMMON_BINARY_DIR}/generated_src/improbable/vector3.h
        ${COMMON_BINARY_DIR}/generated_src/improbable/vector3.cc
        SOURCES ${SCHEMA_SRC})

add_library(Schema STATIC ${SCHEMA_GEN_HDR} ${SCHEMA_GEN_SRC}
        ${COMMON_BINARY_DIR}/generated_src/improbable/standard_library.h
        ${COMMON_BINARY_DIR}/generated_src/improbable/standard_library.cc
        ${COMMON_BINARY_DIR}/generated_src/improbable/vector3.h
        ${COMMON_BINARY_DIR}/generated_src/improbable/vector3.cc)
add_dependencies(Schema schema-gen dependencies)
target_include_directories(Schema SYSTEM PUBLIC ${COMMON_BINARY_DIR}/generated_src ${COMMON_BINARY_DIR}/dependencies/worker_sdk/include)
target_link_libraries(Schema PRIVATE WorkerSdk)