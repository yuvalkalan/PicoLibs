cmake_minimum_required(VERSION 3.13)

project(UniqueID LANGUAGES CXX)

add_library(${PROJECT_NAME} STATIC)

target_sources(${PROJECT_NAME} PRIVATE
        src/UniqueID.cpp
)


target_link_libraries(${PROJECT_NAME} PRIVATE
        pico_stdlib    
        pico_unique_id    
)

target_include_directories(${PROJECT_NAME} PUBLIC include)