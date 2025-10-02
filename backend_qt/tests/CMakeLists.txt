# Copyright (C) 2021-2025 Robin Stuart <rstuart114@gmail.com>
# vim: set ts=4 sw=4 et :

cmake_minimum_required(VERSION 3.10)
project(QZint_tests LANGUAGES CXX)

enable_testing()

if(USE_QT6)
    find_package(Qt6Test REQUIRED)
else()
    find_package(Qt5Test REQUIRED)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)

add_executable(test_qzint test_qzint.cpp)
target_link_libraries(test_qzint PRIVATE QZint Qt${QT_VERSION_MAJOR}::Test)
add_test(NAME qzint COMMAND test_qzint)
if(MSVC)
    string(REPLACE ";" "\\;" env_path "$ENV{PATH}")
    set_tests_properties(qzint PROPERTIES ENVIRONMENT
        "CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR};PATH=${CMAKE_BINARY_DIR}/backend/${CMAKE_BUILD_TYPE}\;${CMAKE_BINARY_DIR}/backend_qt/${CMAKE_BUILD_TYPE}\;${env_path}")
else()
    set_tests_properties(qzint PROPERTIES ENVIRONMENT
        "LD_LIBRARY_PATH=${CMAKE_BINARY_DIR}/backend;PATH=${CMAKE_BINARY_DIR}/backend_qt:$ENV{PATH}")
endif()
