cmake_minimum_required(VERSION 3.18)
project(DomacaUloha_SAD C)

set(CMAKE_C_STANDARD 99)

add_executable(DomacaUloha_SAD main.c)

target_link_libraries(DomacaUloha_SAD pthread)
