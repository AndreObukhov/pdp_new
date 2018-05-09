cmake_minimum_required(VERSION 3.8)
project(Emulator)

set(CMAKE_C_STANDARD 99)


add_executable(Emulator main.c all.h functions.c take.c)