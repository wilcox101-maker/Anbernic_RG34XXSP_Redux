cmake_minimum_required(VERSION 3.16)
project(rg34xxsp-redux C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)

add_executable(rg34xxsp-redux
    src/main.c
    src/grid.c
    src/font.c
    src/platform.c
    src/remap.c
    src/ui.c
)

target_include_directories(rg34xxsp-redux PRIVATE ${SDL2_INCLUDE_DIRS} src)
target_link_libraries(rg34xxsp-redux ${SDL2_LIBRARIES})

if(WIN32)
    target_link_libraries(rg34xxsp-redux SDL2::SDL2main)
endif()
