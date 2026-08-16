cmake_minimum_required(VERSION 3.13)

project(BMI270 LANGUAGES CXX)

add_library(${PROJECT_NAME} STATIC)

target_sources(${PROJECT_NAME} PRIVATE
    src/BMI270.cpp
    src/bmi2.c
    src/bmi270_lib.c
)
target_link_libraries(${PROJECT_NAME} PRIVATE
        pico_stdlib
        hardware_i2c
        )
target_include_directories(${PROJECT_NAME} PUBLIC include)