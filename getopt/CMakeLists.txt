project(zint_bundled_getopt)

add_library(${PROJECT_NAME} STATIC getopt.c getopt1.c)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
