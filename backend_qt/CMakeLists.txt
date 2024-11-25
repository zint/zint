# Copyright (C) 2008 by BogDan Vatra < bogdan@licentia.eu >
# Copyright (C) 2009-2024 Robin Stuart <rstuart114@gmail.com>
# vim: set ts=4 sw=4 et :

project(QZint)

cmake_minimum_required(VERSION 3.10)
set(${PROJECT_NAME}_SRCS qzint.cpp)

if(USE_QT6)
    qt6_wrap_cpp(QZint_SRCS qzint.h)
else()
    qt5_wrap_cpp(QZint_SRCS qzint.h)
endif()

add_library(${PROJECT_NAME} STATIC ${QZint_SRCS})

set_target_properties(${PROJECT_NAME} PROPERTIES  SOVERSION "${ZINT_VERSION_MAJOR}.${ZINT_VERSION_MINOR}"
                                        VERSION ${ZINT_VERSION})

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../backend")

if(ZINT_SHARED)
    target_link_libraries(${PROJECT_NAME} zint)
else()
    target_link_libraries(${PROJECT_NAME} zint-static)
endif()
target_link_libraries(${PROJECT_NAME} Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Gui)

if(ZINT_TEST)
    add_subdirectory(tests)
endif()

install(TARGETS ${PROJECT_NAME} ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES qzint.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} COMPONENT Devel)
