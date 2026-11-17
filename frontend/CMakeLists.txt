cmake_minimum_required(VERSION 3.16)
project(rg34xxsp-redux C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)

# Platform layer: "desktop" (fake values, default) or "stock" (real sysfs on the RG34XXSP).
set(REDUX_PLATFORM desktop CACHE STRING "desktop or stock")
if(REDUX_PLATFORM STREQUAL "stock")
    set(REDUX_PLATFORM_SRC src/platform_stock.c src/engread.c src/engprofile.c src/buildhist.c src/updater.c src/remap_device.c src/evdev_reader.c)
else()
    set(REDUX_PLATFORM_SRC src/platform.c)
endif()

# vsync library for stock apps that tear (frontend/vsync, Dale 2026-11-14 B): built as a shared
# library, then embedded in dmenu.bin, which writes it to /tmp before those launches.
add_library(redux_vsync SHARED vsync/redux_vsync.c)
set_target_properties(redux_vsync PROPERTIES C_VISIBILITY_PRESET default)
target_compile_options(redux_vsync PRIVATE -Wall -Wextra)
target_link_libraries(redux_vsync PRIVATE ${CMAKE_DL_LIBS})
# The device's SimpleTerminal is 32-bit ARM, so the device build embeds an armhf copy built
# separately (CI arm64 job: arm-linux-gnueabihf-gcc) and passed in as REDUX_VSYNC_EMBED. Without
# it, the library built here is embedded (desktop builds and tests).
set(REDUX_VSYNC_EMBED "" CACHE FILEPATH "prebuilt vsync library to embed (armhf for the device)")
if(REDUX_VSYNC_EMBED)
    set(REDUX_VSYNC_IN ${REDUX_VSYNC_EMBED})
    set(REDUX_VSYNC_DEP ${REDUX_VSYNC_EMBED})
else()
    set(REDUX_VSYNC_IN $<TARGET_FILE:redux_vsync>)
    set(REDUX_VSYNC_DEP redux_vsync)
    if(REDUX_PLATFORM STREQUAL "stock")
        message(STATUS "vsync: REDUX_VSYNC_EMBED not set; embedding this machine's library (no effect on the 32-bit stock apps)")
    endif()
endif()
set(REDUX_VSYNC_BLOB ${CMAKE_CURRENT_BINARY_DIR}/vsync_blob.c)
add_custom_command(
    OUTPUT ${REDUX_VSYNC_BLOB}
    COMMAND ${CMAKE_COMMAND} -DIN=${REDUX_VSYNC_IN} -DOUT=${REDUX_VSYNC_BLOB}
            -DNAME=redux_vsync_so -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/embed.cmake
    DEPENDS ${REDUX_VSYNC_DEP} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/embed.cmake
    COMMENT "Embedding the vsync library")

# Lid watcher (F4 step 4): a small program run beside RetroArch games. Built for the same machine
# as dmenu.bin and embedded in it; Redux writes it to /tmp at start.
add_executable(redux_lidwatch lidwatch/redux_lidwatch.c)
target_compile_options(redux_lidwatch PRIVATE -Wall -Wextra)
set(REDUX_LIDWATCH_BLOB ${CMAKE_CURRENT_BINARY_DIR}/lidwatch_blob.c)
add_custom_command(
    OUTPUT ${REDUX_LIDWATCH_BLOB}
    COMMAND ${CMAKE_COMMAND} -DIN=$<TARGET_FILE:redux_lidwatch> -DOUT=${REDUX_LIDWATCH_BLOB}
            -DNAME=redux_lidwatch_bin -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/embed.cmake
    DEPENDS redux_lidwatch ${CMAKE_CURRENT_SOURCE_DIR}/cmake/embed.cmake
    COMMENT "Embedding the lid watcher")

# Heat guard (F5 step 4): runs once per boot in its own session, in the menu and during games.
# Built for the same machine as dmenu.bin and embedded in it; Redux writes it to /tmp and starts it.
add_executable(redux_heatguard heatguard/redux_heatguard.c src/heatguard.c src/engprofile.c)
target_include_directories(redux_heatguard PRIVATE src)
target_compile_options(redux_heatguard PRIVATE -Wall -Wextra)
set(REDUX_HEATGUARD_BLOB ${CMAKE_CURRENT_BINARY_DIR}/heatguard_blob.c)
add_custom_command(
    OUTPUT ${REDUX_HEATGUARD_BLOB}
    COMMAND ${CMAKE_COMMAND} -DIN=$<TARGET_FILE:redux_heatguard> -DOUT=${REDUX_HEATGUARD_BLOB}
            -DNAME=redux_heatguard_bin -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/embed.cmake
    DEPENDS redux_heatguard ${CMAKE_CURRENT_SOURCE_DIR}/cmake/embed.cmake
    COMMENT "Embedding the heat guard")

# Redux Terminal (Dale 2026-11-17 A): a pty + VT100/xterm subset drawn with Redux's fonts. Built for
# the same machine as dmenu.bin and embedded in it; Redux writes it to /tmp at start (src/term.c).
# 16x16 CJK glyphs: term/cjk.hex (Unifont subset, text) -> cjk.bin (tools/cjkconv.py, Python 3
# standard library) -> embedded in redux-term only. The repo keeps no binaries.
find_package(Python3 COMPONENTS Interpreter REQUIRED)
set(REDUX_CJK_BIN ${CMAKE_CURRENT_BINARY_DIR}/cjk.bin)
add_custom_command(
    OUTPUT ${REDUX_CJK_BIN}
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../tools/cjkconv.py blob
            ${CMAKE_CURRENT_SOURCE_DIR}/term/cjk.hex ${REDUX_CJK_BIN}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/term/cjk.hex ${CMAKE_CURRENT_SOURCE_DIR}/../tools/cjkconv.py
    COMMENT "Converting the CJK glyphs")
set(REDUX_CJK_BLOB ${CMAKE_CURRENT_BINARY_DIR}/cjk_blob.c)
add_custom_command(
    OUTPUT ${REDUX_CJK_BLOB}
    COMMAND ${CMAKE_COMMAND} -DIN=${REDUX_CJK_BIN} -DOUT=${REDUX_CJK_BLOB}
            -DNAME=redux_cjk_bin -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/embed.cmake
    DEPENDS ${REDUX_CJK_BIN} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/embed.cmake
    COMMENT "Embedding the CJK glyphs")
add_executable(redux_term term/redux_term.c term/vt.c term/osk.c term/cjk.c ${REDUX_CJK_BLOB} src/font.c src/font_extra.c src/remap_device.c)
target_include_directories(redux_term PRIVATE src term)
target_compile_options(redux_term PRIVATE -Wall -Wextra)
set(REDUX_TERM_BLOB ${CMAKE_CURRENT_BINARY_DIR}/term_blob.c)
add_custom_command(
    OUTPUT ${REDUX_TERM_BLOB}
    COMMAND ${CMAKE_COMMAND} -DIN=$<TARGET_FILE:redux_term> -DOUT=${REDUX_TERM_BLOB}
            -DNAME=redux_term_bin -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/embed.cmake
    DEPENDS redux_term ${CMAKE_CURRENT_SOURCE_DIR}/cmake/embed.cmake
    COMMENT "Embedding the terminal")

# Package catalog for Settings > System > Packages: tools/extras.ini, the same file the PC tool
# reads, embedded so the list on the device and on the PC never differ.
set(REDUX_EXTRAS_BLOB ${CMAKE_CURRENT_BINARY_DIR}/extras_ini_blob.c)
add_custom_command(
    OUTPUT ${REDUX_EXTRAS_BLOB}
    COMMAND ${CMAKE_COMMAND} -DIN=${CMAKE_CURRENT_SOURCE_DIR}/../tools/extras.ini -DOUT=${REDUX_EXTRAS_BLOB}
            -DNAME=redux_extras_ini -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/embed.cmake
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../tools/extras.ini ${CMAKE_CURRENT_SOURCE_DIR}/cmake/embed.cmake
    COMMENT "Embedding the package catalog")

add_executable(rg34xxsp-redux
    src/main.c
    src/grid.c
    src/font.c
    src/font_extra.c
    ${REDUX_PLATFORM_SRC}
    src/remap.c
    src/ui.c
    src/uistyle.c
    src/pkgs.c
    ${REDUX_EXTRAS_BLOB}
    src/netsvc.c
    src/btchip.c
    src/launcher.c
    src/systems.c
    src/romscan.c
    src/wifi.c
    src/stockmenu.c
    src/power.c
    src/powerkeys.c
    src/govrestore.c
    src/lidwatch.c
    ${REDUX_LIDWATCH_BLOB}
    src/heatguard.c
    src/heatguard_install.c
    ${REDUX_HEATGUARD_BLOB}
    src/volume.c
    src/brightness.c
    src/screentimeout.c
    src/sysinfo.c
    src/apps.c
    src/vsync.c
    ${REDUX_VSYNC_BLOB}
    src/ports.c
    src/bootscreen.c
    src/term.c
    ${REDUX_TERM_BLOB}
)
target_include_directories(rg34xxsp-redux PRIVATE src)

if(REDUX_PLATFORM STREQUAL "stock")
    target_compile_definitions(rg34xxsp-redux PRIVATE REDUX_PLATFORM_STOCK)
    target_compile_definitions(redux_term PRIVATE REDUX_PLATFORM_STOCK)
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
target_compile_definitions(redux_term PRIVATE "REDUX_BUILD=\"${REDUX_BUILD}\"")

# Newer SDL2 packages export targets; older ones only set variables.
function(redux_link_sdl target)
    if(TARGET SDL2::SDL2)
        if(TARGET SDL2::SDL2main)
            target_link_libraries(${target} PRIVATE SDL2::SDL2main)
        endif()
        target_link_libraries(${target} PRIVATE SDL2::SDL2)
    else()
        target_include_directories(${target} PRIVATE ${SDL2_INCLUDE_DIRS})
        target_link_libraries(${target} PRIVATE ${SDL2_LIBRARIES})
    endif()
endfunction()
redux_link_sdl(rg34xxsp-redux)
redux_link_sdl(redux_term)

if(MSVC)
    target_compile_options(rg34xxsp-redux PRIVATE /W4 /utf-8)
else()
    target_compile_options(rg34xxsp-redux PRIVATE -Wall -Wextra)
endif()

# Bluetooth chip init against a temp folder (no device): ./test_btchip.
add_executable(test_btchip tests/test_btchip.c src/btchip.c)
target_include_directories(test_btchip PRIVATE src)

# Terminal screen model (no SDL, no pty): ./test_vt, exit code 0 = pass.
add_executable(test_vt tests/test_vt.c term/vt.c term/osk.c term/cjk.c ${REDUX_CJK_BLOB})
target_include_directories(test_vt PRIVATE term)

# Boot Screen manager against a temp folder (no SDL, no device): ./test_bootscreen.
add_executable(test_bootscreen tests/test_bootscreen.c src/bootscreen.c)
target_include_directories(test_bootscreen PRIVATE src)

# Ports known-ports table (no SDL needed): ./test_ports, exit code 0 = pass.
add_executable(test_ports tests/test_ports.c src/romscan.c src/systems.c src/launcher.c src/ports.c)
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

# Power key and lid on event0 (F4 step 1), no device: ./test_powerkeys.
add_executable(test_powerkeys tests/test_powerkeys.c src/powerkeys.c)
target_include_directories(test_powerkeys PRIVATE src)

# CPU governor restore against a fake cpufreq folder (F4 step 2): ./test_govrestore.
add_executable(test_govrestore tests/test_govrestore.c src/govrestore.c)
target_include_directories(test_govrestore PRIVATE src)

# Lid watcher against a file of events and a local UDP socket (F4 step 4): ./test_lidwatch.
add_executable(test_lidwatch tests/test_lidwatch.c src/lidwatch.c ${REDUX_LIDWATCH_BLOB})
target_include_directories(test_lidwatch PRIVATE src)

add_executable(test_brightness tests/test_brightness.c src/brightness.c)
target_include_directories(test_brightness PRIVATE src)

# Build history for Engineering > Update (no SDL, no device): ./test_buildhist.
add_executable(test_buildhist tests/test_buildhist.c src/buildhist.c)
target_include_directories(test_buildhist PRIVATE src)

# Engineering > Update download/install against a local fake release (needs sha256sum): ./test_updater.
add_executable(test_updater tests/test_updater.c src/updater.c src/buildhist.c)
target_include_directories(test_updater PRIVATE src)

# Packages page: catalog parse and the apt command text (no apt run): ./test_pkgs.
add_executable(test_pkgs tests/test_pkgs.c src/pkgs.c ${REDUX_EXTRAS_BLOB})
target_include_directories(test_pkgs PRIVATE src)

# SSH / SMB switches against a temp system.cfg (no systemctl): ./test_netsvc.
add_executable(test_netsvc tests/test_netsvc.c src/netsvc.c)
target_include_directories(test_netsvc PRIVATE src)

# Heat guard decision logic (no SDL, no device): ./test_heatguard.
add_executable(test_heatguard tests/test_heatguard.c src/heatguard.c)
target_include_directories(test_heatguard PRIVATE src)

# Engineering safety net against temp folders (no SDL, no device): ./test_engprofile.
add_executable(test_engprofile tests/test_engprofile.c src/engprofile.c)
target_include_directories(test_engprofile PRIVATE src)

# Engineering Mode read layer against fixtures (no SDL, no device): ./test_engread.
add_executable(test_engread tests/test_engread.c src/engread.c)
target_include_directories(test_engread PRIVATE src)

add_executable(test_sysinfo tests/test_sysinfo.c src/sysinfo.c)
target_include_directories(test_sysinfo PRIVATE src)

# Navigation test (back stack, Engineering tabs). Links the UI with the desktop platform; no window.
add_executable(test_ui_nav tests/test_ui_nav.c src/ui.c src/uistyle.c src/netsvc.c src/btchip.c src/pkgs.c ${REDUX_EXTRAS_BLOB} src/grid.c src/font.c src/font_extra.c src/platform.c src/launcher.c src/systems.c src/romscan.c src/wifi.c src/stockmenu.c src/power.c src/powerkeys.c src/volume.c src/brightness.c src/screentimeout.c src/sysinfo.c src/apps.c src/vsync.c ${REDUX_VSYNC_BLOB} src/ports.c src/bootscreen.c)
target_include_directories(test_ui_nav PRIVATE src)
if(TARGET SDL2::SDL2)
    target_link_libraries(test_ui_nav PRIVATE SDL2::SDL2)
else()
    target_include_directories(test_ui_nav PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(test_ui_nav PRIVATE ${SDL2_LIBRARIES})
endif()

# vsync library: loads it, checks it passes frames through and switches itself off when the
# framebuffer can't wait for vblank (no device needed): ./test_vsync.
add_library(test_fakesdl SHARED tests/fakesdl.c)
add_executable(test_vsync_app tests/test_vsync_app.c)
target_link_libraries(test_vsync_app PRIVATE test_fakesdl ${CMAKE_DL_LIBS})
add_executable(test_vsync tests/test_vsync.c src/vsync.c ${REDUX_VSYNC_BLOB})
target_include_directories(test_vsync PRIVATE src)
add_dependencies(test_vsync redux_vsync test_vsync_app)
target_compile_definitions(test_vsync PRIVATE
    VSYNC_SO="$<TARGET_FILE:redux_vsync>" VSYNC_APP="$<TARGET_FILE:test_vsync_app>"
    VSYNC_EMBEDDED="${REDUX_VSYNC_IN}")
