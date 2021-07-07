# Copyright (C) 2021 Robin Stuart <rstuart114@gmail.com>
# Adapted from qrencode/tests/CMakeLists.txt
# Copyright (C) 2006-2017 Kentaro Fukuchi <kentaro@fukuchi.org>
# vim: set ts=4 sw=4 et :

macro(zint_add_test test_name test_command)
    set(ADDITIONAL_LIBS "${ARGN}" ${LIBRARY_FLAGS})
    add_executable(${test_command} ${test_command}.c)
    target_link_libraries(${test_command} testcommon ${ADDITIONAL_LIBS})
    add_test(${test_name} ${test_command})
    set_tests_properties(${test_name} PROPERTIES ENVIRONMENT "CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}")
    if(ZINT_STATIC)
        add_executable(${test_command}-static ${test_command}.c)
        target_link_libraries(${test_command}-static testcommon-static ${ADDITIONAL_LIBS})
        add_test(${test_name}-static ${test_command}-static)
        set_tests_properties(${test_name}-static PROPERTIES ENVIRONMENT "CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}")
    endif()
endmacro()
