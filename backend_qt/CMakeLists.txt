# Copyright (C) 2008 by BogDan Vatra < bogdan@licentia.eu >
# Copyright (C) 2009-2021 Robin Stuart <rstuart114@gmail.com>
# vim: set ts=4 sw=4 et :

project(QZint)

include_directories(BEFORE "${CMAKE_SOURCE_DIR}/backend")

set(QZint_SRCS qzint.cpp)

if(USE_QT6)
    qt6_wrap_cpp(QZint_SRCS qzint.h)
else()
    qt5_wrap_cpp(QZint_SRCS qzint.h)
endif()

add_library(QZint STATIC ${QZint_SRCS})

set_target_properties(QZint PROPERTIES  SOVERSION "${ZINT_VERSION_MAJOR}.${ZINT_VERSION_MINOR}"
                                        VERSION ${ZINT_VERSION})

add_dependencies(QZint zint)

link_directories("${CMAKE_BINARY_DIR}/backend")

if(USE_QT6)
    target_link_libraries(QZint zint Qt6::Widgets Qt6::Gui)
else()
    target_link_libraries(QZint zint Qt5::Widgets Qt5::Gui)
endif()

install(TARGETS QZint ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES qzint.h DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)
