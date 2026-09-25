cmake_minimum_required(VERSION 3.13)

project(CC1101 LANGUAGES CXX)

add_library(${PROJECT_NAME} STATIC)

target_sources(${PROJECT_NAME} PRIVATE
    src/CC1101.cpp
)
target_link_libraries(${PROJECT_NAME} PRIVATE
        pico_stdlib
        hardware_spi
        # hardware_interp
        Logger
        MultiCore
        HighSPI
)
target_include_directories(${PROJECT_NAME} PUBLIC include)