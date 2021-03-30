project(zint_bundled_getopt)

add_library(zint_bundled_getopt STATIC getopt.c getopt1.c)
target_include_directories(zint_bundled_getopt PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
