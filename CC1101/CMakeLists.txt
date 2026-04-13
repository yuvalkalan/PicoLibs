cmake_minimum_required(VERSION 3.13)

project(CC1101 LANGUAGES CXX)

add_library(${PROJECT_NAME} STATIC)

target_sources(${PROJECT_NAME} PRIVATE
    src/CC1101.cpp
)
target_link_libraries(${PROJECT_NAME} PRIVATE
        pico_stdlib
        hardware_spi
        hardware_i2c
        hardware_interp
        Logger
)
target_include_directories(${PROJECT_NAME} PUBLIC include)