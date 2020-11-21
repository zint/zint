# (c) 2008 by BogDan Vatra < bogdan@licentia.eu >

project(zint_frontend)

set(zint_frontend_SRCS main.c )

include_directories(BEFORE "${CMAKE_SOURCE_DIR}/backend")

add_executable(zint_frontend ${zint_frontend_SRCS})

set_target_properties(zint_frontend PROPERTIES   OUTPUT_NAME "zint")

add_dependencies(zint_frontend zint)

link_directories( "${CMAKE_BINARY_DIR}/backend" )

target_link_libraries(zint_frontend zint)

install(TARGETS zint_frontend DESTINATION "${BIN_INSTALL_DIR}" RUNTIME)

# LOCALEDIR must be set up before including po/CMakeLists.txt
if (DEFINED LOCALE_INSTALL_DIR)
	set(LOCALEDIR "${LOCALE_INSTALL_DIR}")
else(DEFINED LOCALE_INSTALL_DIR)
	set(LOCALEDIR "${CMAKE_INSTALL_PREFIX}/locale")
endif(DEFINED LOCALE_INSTALL_DIR)

add_definitions(-DINSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}")
add_definitions(-DLOCALE_DIR="${LOCALEDIR}")

add_subdirectory(po)
