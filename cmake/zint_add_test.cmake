# Copyright (C) 2021-2024 Robin Stuart <rstuart114@gmail.com>
# Adapted from qrencode/tests/CMakeLists.txt
# Copyright (C) 2006-2017 Kentaro Fukuchi <kentaro@fukuchi.org>
# vim: set ts=4 sw=4 et :

macro(zint_add_test test_name test_command)
    set(ADDITIONAL_LIBS "${ARGN}" ${LIBRARY_FLAGS})
    if(ZINT_SHARED)
        add_executable(${test_command} ${test_command}.c)
        target_link_libraries(${test_command} testcommon ${ADDITIONAL_LIBS})
        if(NOT (ZINT_USE_PNG AND PNG_FOUND))
            target_compile_definitions(${test_command} PRIVATE ZINT_NO_PNG)
        endif()
        add_test(${test_name} ${test_command})
        if(MSVC)
            set_tests_properties(${test_name} PROPERTIES ENVIRONMENT
                "CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR};PATH=${CMAKE_BINARY_DIR}/backend/${CMAKE_BUILD_TYPE}\;${CMAKE_BINARY_DIR}/frontend/${CMAKE_BUILD_TYPE}\;$ENV{PATH}")
        else()
            set_tests_properties(${test_name} PROPERTIES ENVIRONMENT
                "CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR};LD_LIBRARY_PATH=${CMAKE_BINARY_DIR}/backend;PATH=${CMAKE_BINARY_DIR}/frontend:$ENV{PATH}")
        endif()
    endif()
    if(ZINT_STATIC)
        add_executable(${test_command}-static ${test_command}.c)
        target_link_libraries(${test_command}-static testcommon-static ${ADDITIONAL_LIBS})
        if(NOT (ZINT_USE_PNG AND PNG_FOUND))
            target_compile_definitions(${test_command}-static PRIVATE ZINT_NO_PNG)
        endif()
        add_test(${test_name}-static ${test_command}-static)
        if(MSVC)
            set_tests_properties(${test_name}-static PROPERTIES ENVIRONMENT
                "CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR};PATH=${CMAKE_BINARY_DIR}/backend/${CMAKE_BUILD_TYPE}\;${CMAKE_BINARY_DIR}/frontend/${CMAKE_BUILD_TYPE}\;$ENV{PATH}")
        else()
            set_tests_properties(${test_name}-static PROPERTIES ENVIRONMENT
                "CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR};LD_LIBRARY_PATH=${CMAKE_BINARY_DIR}/backend;PATH=${CMAKE_BINARY_DIR}/frontend:$ENV{PATH}")
        endif()
    endif()
endmacro()
