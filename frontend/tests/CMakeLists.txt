# Copyright (C) 2020 Robin Stuart <rstuart114@gmail.com>
# Adapted from qrencode/tests/CMakeLists.txt
# Copyright (C) 2006-2017 Kentaro Fukuchi <kentaro@fukuchi.org>
# vim: set ts=4 sw=4 et :

cmake_minimum_required(VERSION 3.9)
project(zint_frontend_tests)

enable_testing()

macro(zint_add_test test_name test_command)
    set(ADDITIONAL_LIBS "${ARGN}" ${LIBRARY_FLAGS})
    add_executable(${test_command} ${test_command}.c)
    target_link_libraries(${test_command} testcommon ${ADDITIONAL_LIBS})
    add_test(${test_name} ${test_command})
    if(ZINT_STATIC)
        add_executable(${test_command}-static ${test_command}.c)
        target_link_libraries(${test_command}-static testcommon-static ${ADDITIONAL_LIBS})
        add_test(${test_name}-static ${test_command}-static)
    endif()
endmacro()

zint_add_test(args test_args)
