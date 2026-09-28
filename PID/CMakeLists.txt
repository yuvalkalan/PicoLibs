cmake_minimum_required(VERSION 3.13)

project(PID LANGUAGES CXX)

add_library(${PROJECT_NAME} STATIC)

target_sources(${PROJECT_NAME} PRIVATE
        src/PID.cpp
)


target_link_libraries(${PROJECT_NAME} PRIVATE
        pico_stdlib
        hardware_pio
        hardware_dma
        hardware_i2c
        DShot600
        BMI270
        ESC
)

target_include_directories(${PROJECT_NAME} PUBLIC include)