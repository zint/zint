# Copyright (C) 2008 by BogDan Vatra < bogdan@licentia.eu >
# Copyright (C) 2009-2021 Robin Stuart <rstuart114@gmail.com>
# vim: set ts=4 sw=4 et :

project(QZint)

set(${PROJECT_NAME}_SRCS qzint.cpp)

if(USE_QT6)
    qt6_wrap_cpp(QZint_SRCS qzint.h)
else()
    qt5_wrap_cpp(QZint_SRCS qzint.h)
endif()

add_library(${PROJECT_NAME} STATIC ${QZint_SRCS})

set_target_properties(${PROJECT_NAME} PROPERTIES  SOVERSION "${ZINT_VERSION_MAJOR}.${ZINT_VERSION_MINOR}"
                                        VERSION ${ZINT_VERSION})

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/backend")

target_link_libraries(${PROJECT_NAME} zint Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Gui)


install(TARGETS ${PROJECT_NAME} ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES qzint.h DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)
