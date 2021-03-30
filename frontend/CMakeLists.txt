# (c) 2008 by BogDan Vatra < bogdan@licentia.eu >

project(zint_frontend)

set(zint_frontend_SRCS main.c)

include_directories(BEFORE "${CMAKE_SOURCE_DIR}/backend")

add_executable(zint_frontend ${zint_frontend_SRCS})

set_target_properties(zint_frontend PROPERTIES   OUTPUT_NAME "zint")

target_link_libraries(zint_frontend zint)
if(NOT HAVE_GETOPT)
    target_link_libraries(zint_frontend zint_bundled_getopt)
endif(NOT HAVE_GETOPT)

install(TARGETS zint_frontend DESTINATION "${BIN_INSTALL_DIR}" RUNTIME)

if(ZINT_TEST)
    add_subdirectory(tests)
endif(ZINT_TEST)
