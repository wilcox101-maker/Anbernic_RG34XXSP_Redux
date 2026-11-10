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
    src/launcher.c
    src/systems.c
)
target_include_directories(rg34xxsp-redux PRIVATE src)

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
