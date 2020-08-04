# Copyright (C) 2020 Robin Stuart <rstuart114@gmail.com>
# Adapted from qrencode/tests/CMakeLists.txt
# Copyright (C) 2006-2017 Kentaro Fukuchi <kentaro@fukuchi.org>
# vim: set ts=4 sw=4 et :

cmake_minimum_required(VERSION 3.9)

enable_testing()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

set(ZINT_DEBUG FALSE CACHE BOOL "Set debug compile flag")
set(ZINT_SANITIZE FALSE CACHE BOOL "Set sanitize compile/link flags")

find_package(LibZint REQUIRED)
find_package(PNG REQUIRED)

if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    if(ZINT_DEBUG)
        add_compile_options("-g")
    endif()
    if(ZINT_SANITIZE)
        add_compile_options("-fsanitize=undefined")
        add_compile_options("-fsanitize=address")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=undefined -fsanitize=address")
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
