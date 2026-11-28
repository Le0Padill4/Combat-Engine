cmake_minimum_required(VERSION 3.16)
project(CombatEngineIntegrante3 LANGUAGES CXX)

# Independiente hasta que el equipo acuerde cambios en el CMakeLists.txt raiz.
# Solo cabeceras de src/headers/i3: no depende de main.cpp ni de Personaje.
get_filename_component(COMBAT_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
add_executable(i3_tests i3_tests.cpp)
foreach(target IN ITEMS i3_tests)
    target_compile_features(${target} PRIVATE cxx_std_17)
    target_include_directories(${target} PRIVATE "${COMBAT_ROOT}/src/headers")
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /WX /utf-8)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
endforeach()
target_include_directories(i3_tests PRIVATE "${CMAKE_CURRENT_LIST_DIR}")
enable_testing()
add_test(NAME i3_tests COMMAND i3_tests)
