cmake_minimum_required(VERSION 3.10)
project(callback)

set(COMPILE_OPTIONS "")
if (MSVC)
    list(APPEND COMPILE_OPTIONS "/W4")
else()
    list(APPEND COMPILE_OPTIONS "-Wall")
    list(APPEND COMPILE_OPTIONS "-Wextra")
    list(APPEND COMPILE_OPTIONS "-Wpedantic")
    list(APPEND COMPILE_OPTIONS "-Wconversion")
    # Use an older standard.
    list(APPEND COMPILE_OPTIONS "-std=c++98")
endif()

add_executable(
    callback
    main.cpp)

target_compile_options(callback PRIVATE "${COMPILE_OPTIONS}")
