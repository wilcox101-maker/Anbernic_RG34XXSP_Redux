cmake_minimum_required(VERSION 3.16)
project(rg34xxsp-redux C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)

# Platform layer: "desktop" (fake values, default) or "stock" (real sysfs on the RG34XXSP).
set(REDUX_PLATFORM desktop CACHE STRING "desktop or stock")
if(REDUX_PLATFORM STREQUAL "stock")
    set(REDUX_PLATFORM_SRC src/platform_stock.c src/remap_device.c src/evdev_reader.c)
else()
    set(REDUX_PLATFORM_SRC src/platform.c)
endif()

add_executable(rg34xxsp-redux
    src/main.c
    src/grid.c
    src/font.c
    ${REDUX_PLATFORM_SRC}
    src/remap.c
    src/ui.c
    src/launcher.c
    src/systems.c
)
target_include_directories(rg34xxsp-redux PRIVATE src)

if(REDUX_PLATFORM STREQUAL "stock")
    target_compile_definitions(rg34xxsp-redux PRIVATE REDUX_PLATFORM_STOCK)
endif()

# Newer SDL2 packages export targets; older ones only set variables.
if(TARGET SDL2::SDL2)
    if(TARGET SDL2::SDL2main)
        target_link_libraries(rg34xxsp-redux PRIVATE SDL2::SDL2main)
    endif()
    target_link_libraries(rg34xxsp-redux PRIVATE SDL2::SDL2)
else()
    target_include_directories(rg34xxsp-redux PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(rg34xxsp-redux PRIVATE ${SDL2_LIBRARIES})
endif()

if(MSVC)
    target_compile_options(rg34xxsp-redux PRIVATE /W4 /utf-8)
else()
    target_compile_options(rg34xxsp-redux PRIVATE -Wall -Wextra)
endif()

# Desktop test for the F2 launcher (no SDL needed): run ./test_launcher, exit code 0 = pass.
add_executable(test_launcher tests/test_launcher.c src/launcher.c)
target_include_directories(test_launcher PRIVATE src)

# Device remap test (no SDL needed).
add_executable(test_remap_device tests/test_remap_device.c src/remap_device.c)
target_include_directories(test_remap_device PRIVATE src)
