# Copyright (C) 2020 Robin Stuart <rstuart114@gmail.com>
# Adapted from qrencode/tests/CMakeLists.txt
# Copyright (C) 2006-2017 Kentaro Fukuchi <kentaro@fukuchi.org>
# vim: set ts=4 sw=4 et :

cmake_minimum_required(VERSION 3.9)
project(zint_frontend_tests)

enable_testing()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

set(ZINT_DEBUG FALSE CACHE BOOL "Set debug compile flag")
set(ZINT_SANITIZE FALSE CACHE BOOL "Set sanitize compile/link flags")

find_package(LibZint REQUIRED)
find_package(PNG REQUIRED)

include(CheckCCompilerFlag)

check_c_compiler_flag(-Wall C_COMPILER_FLAG_WALL)
if(C_COMPILER_FLAG_WALL)
    add_compile_options("-Wall")
endif()

check_c_compiler_flag(-Wextra C_COMPILER_FLAG_WEXTRA)
if(C_COMPILER_FLAG_WEXTRA)
    add_compile_options("-Wextra")
endif()

if(ZINT_DEBUG)
    check_c_compiler_flag(-g C_COMPILER_FLAG_G)
    if(C_COMPILER_FLAG_G)
        add_compile_options("-g")
    endif()
endif()

if(ZINT_SANITIZE)
    # check_c_compiler_flag fails for -fsanitize
    if(${CMAKE_C_COMPILER_ID} STREQUAL "GNU")
        add_compile_options("-fsanitize=undefined")
        add_compile_options("-fsanitize=address")
        link_libraries("-fsanitize=undefined -fsanitize=address")
    endif()
endif()

include_directories(../../backend/tests)
add_library(testcommon STATIC IMPORTED GLOBAL)
set_target_properties(testcommon PROPERTIES IMPORTED_LOCATION "../../../backend/tests/build/libtestcommon.a")

macro(zint_add_test test_name test_command)
    set(ADDITIONAL_LIBS "${ARGN}" ${LIBRARY_FLAGS})
    add_executable(${test_command} ${test_command}.c)
    target_link_libraries(${test_command} testcommon ZINT::ZINT ${PNG_LIBRARIES} ${ADDITIONAL_LIBS})
    add_test(${test_name} ${test_command})
endmacro()

zint_add_test(args, test_args)
