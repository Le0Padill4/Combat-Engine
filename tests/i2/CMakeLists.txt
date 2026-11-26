cmake_minimum_required(VERSION 3.16)
project(CombatI2Tests LANGUAGES CXX)

enable_testing()
add_executable(test_turno
    test_turno.cpp
    ../../src/combat/turno.cpp
    ../../src/personajes/personaje.cpp
    ../../src/personajes/leo1.cpp
)
target_compile_features(test_turno PRIVATE cxx_std_17)
target_include_directories(test_turno PRIVATE ../../src/headers ../../src/combat)
add_test(NAME turno COMMAND test_turno)
