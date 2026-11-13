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
    src/romscan.c
    src/wifi.c
    src/stockmenu.c
    src/power.c
    src/volume.c
    src/brightness.c
    src/screentimeout.c
    src/sysinfo.c
    src/apps.c
    src/ports.c
    src/cardscripts.c
    src/bootscreen.c
)
target_include_directories(rg34xxsp-redux PRIVATE src)

if(REDUX_PLATFORM STREQUAL "stock")
    target_compile_definitions(rg34xxsp-redux PRIVATE REDUX_PLATFORM_STOCK)
endif()

# Short git SHA so a deployed dmenu.bin can be identified. "dev" if git is missing.
# CI passes -DREDUX_BUILD=<sha>: the arm64 container has no git.
if(NOT DEFINED REDUX_BUILD)
    find_package(Git QUIET)
    set(REDUX_BUILD "dev")
    if(GIT_FOUND)
        execute_process(
            COMMAND ${GIT_EXECUTABLE} rev-parse --short=7 HEAD
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/..
            OUTPUT_VARIABLE REDUX_BUILD
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
            RESULT_VARIABLE REDUX_GIT_RC
        )
        if(NOT REDUX_GIT_RC EQUAL 0 OR REDUX_BUILD STREQUAL "")
            set(REDUX_BUILD "dev")
        endif()
    endif()
endif()
target_compile_definitions(rg34xxsp-redux PRIVATE "REDUX_BUILD=\"${REDUX_BUILD}\"")

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

# Boot Screen manager against a temp folder (no SDL, no device): ./test_bootscreen.
add_executable(test_bootscreen tests/test_bootscreen.c src/bootscreen.c)
target_include_directories(test_bootscreen PRIVATE src)

# Ports known-ports table (no SDL needed): ./test_ports, exit code 0 = pass.
add_executable(test_ports tests/test_ports.c src/romscan.c src/systems.c src/launcher.c src/ports.c src/cardscripts.c)
target_include_directories(test_ports PRIVATE src)

# Desktop test for the F2 launcher (no SDL needed): run ./test_launcher, exit code 0 = pass.
add_executable(test_launcher tests/test_launcher.c src/launcher.c)
target_include_directories(test_launcher PRIVATE src)

# Device remap test (no SDL needed).
add_executable(test_remap_device tests/test_remap_device.c src/remap_device.c)
target_include_directories(test_remap_device PRIVATE src)

# Wi-Fi argv and parse test (no SDL, no radio).
add_executable(test_wifi tests/test_wifi.c src/wifi.c)
target_include_directories(test_wifi PRIVATE src)

add_executable(test_stockmenu tests/test_stockmenu.c src/stockmenu.c)
add_executable(test_volume tests/test_volume.c src/volume.c)
target_include_directories(test_volume PRIVATE src)
target_include_directories(test_stockmenu PRIVATE src)

add_executable(test_screentimeout tests/test_screentimeout.c src/screentimeout.c)
target_include_directories(test_screentimeout PRIVATE src)

add_executable(test_brightness tests/test_brightness.c src/brightness.c)
target_include_directories(test_brightness PRIVATE src)

add_executable(test_sysinfo tests/test_sysinfo.c src/sysinfo.c)
target_include_directories(test_sysinfo PRIVATE src)

# Navigation test (back stack, Engineering tabs). Links the UI with the desktop platform; no window.
add_executable(test_ui_nav tests/test_ui_nav.c src/ui.c src/grid.c src/font.c src/platform.c src/launcher.c src/systems.c src/romscan.c src/wifi.c src/stockmenu.c src/power.c src/volume.c src/brightness.c src/screentimeout.c src/sysinfo.c src/apps.c src/ports.c src/cardscripts.c src/bootscreen.c)
target_include_directories(test_ui_nav PRIVATE src)
if(TARGET SDL2::SDL2)
    target_link_libraries(test_ui_nav PRIVATE SDL2::SDL2)
else()
    target_include_directories(test_ui_nav PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(test_ui_nav PRIVATE ${SDL2_LIBRARIES})
endif()
