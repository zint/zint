# (c) 2008 by BogDan Vatra < bogdan@licentia.eu >

project(QZint)

include_directories(BEFORE "${CMAKE_SOURCE_DIR}/backend" )

set(QZint_SRCS qzint.cpp)

add_library(QZint SHARED ${QZint_SRCS})

set_target_properties(QZint PROPERTIES	SOVERSION "${ZINT_VERSION_MAJOR}.${ZINT_VERSION_MINOR}"
                                        VERSION ${ZINT_VERSION})

target_link_libraries(QZint zint ${QT_QTGUI_LIBRARY} 
        ${QT_QTCORE_LIBRARY} )

install(TARGETS QZint  ${INSTALL_TARGETS_DEFAULT_ARGS} )
install(FILES qzint.h DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)
