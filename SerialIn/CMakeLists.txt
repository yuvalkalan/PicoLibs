cmake_minimum_required(VERSION 3.13)

project(SerialIn LANGUAGES CXX)

add_library(${PROJECT_NAME} STATIC)

target_sources(${PROJECT_NAME} PRIVATE
        src/SerialIn.cpp
)


target_link_libraries(${PROJECT_NAME} PRIVATE
        pico_stdlib
        pico_bootrom
        hardware_watchdog
        pico_stdio_usb
)


target_include_directories(${PROJECT_NAME} PUBLIC include)