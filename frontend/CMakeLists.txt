# Copyright (C) 2008 by BogDan Vatra < bogdan@licentia.eu >
# Copyright (C) 2009-2021 Robin Stuart <rstuart114@gmail.com>

project(zint_frontend)

set(${PROJECT_NAME}_SRCS main.c)

add_executable(${PROJECT_NAME} ${zint_frontend_SRCS})

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/backend")

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "zint")

target_link_libraries(${PROJECT_NAME} zint)
if(NOT HAVE_GETOPT)
    target_link_libraries(${PROJECT_NAME} zint_bundled_getopt)
endif()

install(TARGETS ${PROJECT_NAME} DESTINATION "${BIN_INSTALL_DIR}" RUNTIME)

if(ZINT_TEST)
    add_subdirectory(tests)
endif()
