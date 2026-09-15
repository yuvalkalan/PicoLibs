cmake_minimum_required(VERSION 3.13)

project(ADS1115 LANGUAGES CXX)

add_library(${PROJECT_NAME} STATIC)

target_sources(${PROJECT_NAME} PRIVATE
    src/ADS1115.cpp
)
target_link_libraries(${PROJECT_NAME} PRIVATE
        pico_stdlib
        hardware_i2c
        HighI2C
)
target_include_directories(${PROJECT_NAME} PUBLIC include)