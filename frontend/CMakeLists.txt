# Copyright (C) 2008 by BogDan Vatra < bogdan@licentia.eu >
# Copyright (C) 2009-2024 Robin Stuart <rstuart114@gmail.com>
# vim: set ts=4 sw=4 et :

project(zint_frontend)

set(${PROJECT_NAME}_SRCS main.c)

add_executable(${PROJECT_NAME} ${zint_frontend_SRCS})

if(WIN32)
    target_sources(${PROJECT_NAME} PRIVATE zint.rc)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../backend")

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "zint")

if(ZINT_SHARED)
    target_link_libraries(${PROJECT_NAME} zint)
else()
    target_link_libraries(${PROJECT_NAME} zint-static)
endif()
if(NOT HAVE_GETOPT_LONG_ONLY)
    target_link_libraries(${PROJECT_NAME} zint_bundled_getopt)
endif()

install(TARGETS ${PROJECT_NAME} DESTINATION "${BIN_INSTALL_DIR}" RUNTIME)
if(UNIX)
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/../docs/zint.1" DESTINATION "${MAN_INSTALL_DIR}/man1" COMPONENT doc)
endif()

if(ZINT_TEST)
    add_subdirectory(tests)
endif()
