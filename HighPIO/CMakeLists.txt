cmake_minimum_required(VERSION 3.13)

project(HighPIO LANGUAGES CXX)

add_library(${PROJECT_NAME} STATIC)

target_sources(${PROJECT_NAME} PRIVATE
    src/HighPIO.cpp
)
target_link_libraries(${PROJECT_NAME} PRIVATE
        pico_stdlib
        hardware_pio
)
target_include_directories(${PROJECT_NAME} PUBLIC include)