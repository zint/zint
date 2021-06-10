# Copyright (C) 2008 by BogDan Vatra < bogdan@licentia.eu >
# Copyright (C) 2009-2021 Robin Stuart <rstuart114@gmail.com>

project(zint_frontend)

set(zint_frontend_SRCS main.c)

include_directories(BEFORE "${CMAKE_SOURCE_DIR}/backend")

add_executable(zint_frontend ${zint_frontend_SRCS})

set_target_properties(zint_frontend PROPERTIES OUTPUT_NAME "zint")

target_link_libraries(zint_frontend zint)
if(NOT HAVE_GETOPT)
    target_link_libraries(zint_frontend zint_bundled_getopt)
endif()

install(TARGETS zint_frontend DESTINATION "${BIN_INSTALL_DIR}" RUNTIME)

if(ZINT_TEST)
    add_subdirectory(tests)
endif()
