cmake_minimum_required(VERSION 3.13)

project(ESC LANGUAGES CXX)

add_library(${PROJECT_NAME} STATIC)

target_sources(${PROJECT_NAME} PRIVATE
    src/ESC.cpp
)
target_link_libraries(${PROJECT_NAME} PRIVATE
        pico_stdlib
        hardware_pio
        hardware_dma
        DShot600
)
target_include_directories(${PROJECT_NAME} PUBLIC include)

# # add example file here
# add_executable(ESC_example examples/ESC_example.cpp)

# # Links the ESC library (and its PUBLIC dependencies/includes) to the example
# target_link_libraries(ESC_example PRIVATE ${PROJECT_NAME})

# # Generates the .uf2, .bin, and .hex files for the Pico
# pico_add_extra_outputs(ESC_example)