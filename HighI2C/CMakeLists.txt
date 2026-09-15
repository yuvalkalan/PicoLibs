cmake_minimum_required(VERSION 3.13)

project(HighI2C LANGUAGES CXX)

add_library(${PROJECT_NAME} STATIC)

target_sources(${PROJECT_NAME} PRIVATE
    src/HighI2C.cpp
)
target_link_libraries(${PROJECT_NAME} PRIVATE
        pico_stdlib
        hardware_i2c
)
target_include_directories(${PROJECT_NAME} PUBLIC include)