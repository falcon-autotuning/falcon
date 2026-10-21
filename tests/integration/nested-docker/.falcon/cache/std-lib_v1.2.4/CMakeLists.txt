cmake_minimum_required(VERSION 3.20)
project(falcon_stdlib LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Find all required packages
find_package(Armadillo CONFIG REQUIRED)
find_package(ensmallen CONFIG REQUIRED)
find_package(falcon-core CONFIG REQUIRED COMPONENTS c-api)
find_package(falcon-typing CONFIG REQUIRED)
find_package(falcon-routine CONFIG REQUIRED)
find_package(falcon-database CONFIG REQUIRED)
find_package(falcon-comms CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(HighFive CONFIG REQUIRED)

# Common base interface target for all FFI wrappers
add_library(falcon_wrapper_base INTERFACE)
target_compile_definitions(falcon_wrapper_base INTERFACE elements=items)
target_compile_features(falcon_wrapper_base INTERFACE cxx_std_20)
target_link_libraries(falcon_wrapper_base INTERFACE
    falcon-core::falcon-core
    falcon-core::falcon-core-c-api
    falcon::falcon-typing
    falcon::falcon-routine
    falcon::falcon-database
    falcon::falcon-comms
    yaml-cpp::yaml-cpp
    spdlog::spdlog
    fmt::fmt
    HighFive
)

# Native Clang/GCC Precompiled Headers: parsed ONCE and shared across all 45 wrappers
target_precompile_headers(falcon_wrapper_base INTERFACE
    <string>
    <vector>
    <memory>
    <map>
    <falcon-typing/FFIHelpers.hpp>
)

# Discover all wrapper sources across the workspace
file(GLOB_RECURSE WRAPPER_SRCS
    RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
    "*-wrapper.cpp"
)

# Exclude vcpkg directories and cache directories
list(FILTER WRAPPER_SRCS EXCLUDE REGEX "^vcpkg.*")
list(FILTER WRAPPER_SRCS EXCLUDE REGEX ".*\\.falcon/.*")
list(FILTER WRAPPER_SRCS EXCLUDE REGEX "^build.*")

foreach(SRC IN LISTS WRAPPER_SRCS)
    get_filename_component(SRC_DIR "${SRC}" DIRECTORY)
    get_filename_component(SRC_NAME_WE "${SRC}" NAME_WE)

    # Generate a unique target name (e.g., falconCore_physics_config_config-wrapper)
    string(REPLACE "/" "_" TARGET_NAME "${SRC_DIR}_${SRC_NAME_WE}")
    string(REPLACE "-" "_" TARGET_NAME "${TARGET_NAME}")
    string(REGEX REPLACE "^\\._" "" TARGET_NAME "${TARGET_NAME}")

    add_library(${TARGET_NAME} SHARED "${CMAKE_CURRENT_SOURCE_DIR}/${SRC}")
    target_link_libraries(${TARGET_NAME} PRIVATE falcon_wrapper_base)

    # Output directly to the package's build/ folder so .fal imports and falcon.yml resolve it
    set_target_properties(${TARGET_NAME} PROPERTIES
        PREFIX ""
        OUTPUT_NAME "${SRC_NAME_WE}"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${SRC_DIR}/build"
    )
endforeach()
