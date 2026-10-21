cmake_minimum_required(VERSION 3.25)
project(wellness_plugins C CXX)

find_package(instrument-data CONFIG REQUIRED)
find_package(instrument-plugin-api CONFIG REQUIRED)

add_library(plugin-helpers INTERFACE)

target_include_directories(plugin-helpers
  INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/plugin-helpers.h
)

add_instrument_plugin(mock_multimeter
    SOURCES mock-multimeter.c
    LINK_LIBRARIES
        instrument-data::instrument-data
        plugin-helpers
)

add_instrument_plugin(mock_voltage_source
    SOURCES mock-voltage-source.c
    LINK_LIBRARIES
        plugin-helpers
)
