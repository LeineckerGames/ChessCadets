cmake_minimum_required(VERSION 3.10)
project(main)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(core STATIC
        bitboard.cpp
        model/castling.cpp
        model/color.cpp
        evaluation.cpp
        notation.cpp
        model/file.cpp
        model/move.cpp
        movegenerator.cpp
        movelist.cpp
        perft.cpp
        model/piece.cpp
        model/piecetype.cpp
        position.cpp
        pulse.cpp
        model/rank.cpp
        search.cpp
        model/square.cpp
        model/value.cpp
        )

add_executable(pulse main.cpp)
set_target_properties(pulse PROPERTIES OUTPUT_NAME "pulse-cpp-${PLATFORM_SUFFIX}-${pulse_VERSION}")

find_package(Threads REQUIRED)
target_link_libraries(pulse core Threads::Threads)

install(TARGETS pulse DESTINATION .)
