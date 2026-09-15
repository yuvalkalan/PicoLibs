cmake_minimum_required(VERSION 3.13)

project(Dshot600 LANGUAGES CXX)

add_library(${PROJECT_NAME} STATIC)

pico_generate_pio_header(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/src/Dshot600.pio)

target_sources(${PROJECT_NAME} PRIVATE
    src/Dshot600.cpp
)
target_link_libraries(${PROJECT_NAME} PRIVATE
        pico_stdlib  
        hardware_pio
        hardware_clocks
)
target_include_directories(${PROJECT_NAME} PUBLIC include)