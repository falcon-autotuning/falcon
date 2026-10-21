cmake_minimum_required(VERSION 3.25)

find_package(instrument-data CONFIG REQUIRED)
project(wellness_plugins C)

add_library(plugin-helpers INTERFACE)

target_include_directories(plugin-helpers
  INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/plugin-helpers.h
)

find_package(instrument-controller CONFIG REQUIRED)

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
