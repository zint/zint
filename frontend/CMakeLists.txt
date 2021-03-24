# (c) 2008 by BogDan Vatra < bogdan@licentia.eu >

project(zint_frontend)

include(CheckFunctionExists)

check_function_exists(getopt HAVE_GETOPT)

set(zint_frontend_SRCS main.c)

if(NOT HAVE_GETOPT)
    set(zint_frontend_SRCS ${zint_frontend_SRCS} getopt.c getopt1.c)
endif(NOT HAVE_GETOPT)

include_directories(BEFORE "${CMAKE_SOURCE_DIR}/backend")

add_executable(zint_frontend ${zint_frontend_SRCS})

set_target_properties(zint_frontend PROPERTIES   OUTPUT_NAME "zint")

link_directories( "${CMAKE_BINARY_DIR}/backend" )

target_link_libraries(zint_frontend zint)

install(TARGETS zint_frontend DESTINATION "${BIN_INSTALL_DIR}" RUNTIME)
