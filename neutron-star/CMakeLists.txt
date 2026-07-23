cmake_minimum_required(VERSION 3.10)
project(NeutronStar VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(include)


add_executable(tov_sim 
    app/main.cpp 
    src/eos.cpp 
    src/rk4.cpp 
    src/tov.cpp
)