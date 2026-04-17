cmake_minimum_required(VERSION 3.13)

project(ConnectCC1101 LANGUAGES CXX)

add_library(${PROJECT_NAME} STATIC)


target_sources(${PROJECT_NAME} PRIVATE
    src/ConnectCC1101.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        pico_stdlib
        pico_rand
        hardware_spi
        hardware_i2c
        hardware_interp
        CC1101
        Logger
        MultiCore
)
target_include_directories(${PROJECT_NAME} PUBLIC include)