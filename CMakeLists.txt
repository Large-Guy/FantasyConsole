cmake_minimum_required(VERSION 3.27)
project(FakeOS C)

set(CMAKE_C_STANDARD 11)

find_package(sdl2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# add -rpath /Library/Frameworks to the linker flags
if (APPLE)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rpath /Library/Frameworks")
endif ()

add_executable(FakeOS src/main.c
        src/rendering.c
        src/rendering.h
        src/asm.c
        src/asm.h)

target_link_libraries(FakeOS ${SDL2_LIBRARIES})
