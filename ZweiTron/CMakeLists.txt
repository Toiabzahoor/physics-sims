cmake_minimum_required(VERSION 3.14)
project(ZweiTron VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(include)

include(FetchContent)
FetchContent_Declare(
    raylib
    URL https://github.com/raysan5/raylib/archive/refs/tags/5.0.tar.gz
)
FetchContent_MakeAvailable(raylib)

add_executable(ZweiTron
     app/main.cpp
     src/eos.cpp
     src/rk4.cpp
     src/tov.cpp
     src/blackhole.cpp
     src/renderer.cpp
     src/star_physics.cpp
     src/jets.cpp
)

target_link_libraries(ZweiTron PRIVATE raylib)

if(WIN32)
    target_link_option(ZweiTron PRIVATE -static -static-libgcc -static-libstdc++)
endif()
