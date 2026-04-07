cmake_minimum_required(VERSION 3.13)

project(Logger LANGUAGES CXX)

add_library(${PROJECT_NAME} STATIC)

target_sources(${PROJECT_NAME} PRIVATE
        src/Logger.cpp
)


target_link_libraries(${PROJECT_NAME} PRIVATE
        pico_stdlib
)


pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)


target_include_directories(${PROJECT_NAME} PUBLIC include)