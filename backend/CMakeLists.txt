# Copyright (C) 2008 by BogDan Vatra < bogdan@licentia.eu >
# Copyright (C) 2009-2024 Robin Stuart <rstuart114@gmail.com>
# vim: set ts=4 sw=4 et :

cmake_minimum_required(VERSION 3.10)
project(zint)

if(ZINT_USE_PNG)
    find_package(PNG)
endif()

set(zint_COMMON_SRCS common.c library.c large.c reedsol.c gs1.c eci.c filemem.c general_field.c)
set(zint_ONEDIM_SRCS bc412.c code.c code128.c 2of5.c upcean.c telepen.c medical.c plessey.c rss.c dxfilmedge.c)
set(zint_POSTAL_SRCS postal.c auspost.c imail.c mailmark.c)
set(zint_TWODIM_SRCS code16k.c codablock.c dmatrix.c pdf417.c qr.c maxicode.c composite.c aztec.c code49.c code1.c gridmtx.c hanxin.c dotcode.c ultra.c)
if(ZINT_USE_PNG AND PNG_FOUND)
    set(zint_OUTPUT_SRCS vector.c ps.c svg.c emf.c bmp.c pcx.c gif.c png.c tif.c raster.c output.c)
else()
    set(zint_OUTPUT_SRCS vector.c ps.c svg.c emf.c bmp.c pcx.c gif.c tif.c raster.c output.c)
endif()
set(zint_SRCS ${zint_OUTPUT_SRCS} ${zint_COMMON_SRCS} ${zint_ONEDIM_SRCS} ${zint_POSTAL_SRCS} ${zint_TWODIM_SRCS})

if(ZINT_SHARED)
    add_library(zint SHARED ${zint_SRCS})

    if(WIN32)
        target_sources(${PROJECT_NAME} PRIVATE libzint.rc)
    endif()
endif()

if(ZINT_STATIC)
    add_library(zint-static STATIC ${zint_SRCS})
    if(WIN32)
        set_target_properties(zint-static PROPERTIES OUTPUT_NAME zint-static)
    else()
        set_target_properties(zint-static PROPERTIES OUTPUT_NAME zint)
    endif()
endif()

function(zint_target_link_libraries library)
    if(ZINT_SHARED)
        target_link_libraries(zint ${library})
    endif()
    if(ZINT_STATIC)
        target_link_libraries(zint-static ${library})
    endif()
endfunction()

function(zint_target_compile_definitions scope definition)
    if(ZINT_SHARED)
        target_compile_definitions(zint ${scope} ${definition})
    endif()
    if(ZINT_STATIC)
        target_compile_definitions(zint-static ${scope} ${definition})
    endif()
endfunction()

function(zint_target_compile_options scope option)
    if(ZINT_SHARED)
        target_compile_options(zint ${scope} ${option})
    endif()
    if(ZINT_STATIC)
        target_compile_options(zint-static ${scope} ${option})
    endif()
endfunction()

function(zint_target_include_directories)
    if(ZINT_SHARED)
        target_include_directories(zint ${ARGN})
    endif()
    if(ZINT_STATIC)
        target_include_directories(zint-static ${ARGN})
    endif()
endfunction()

if(ZINT_SHARED)
    set_target_properties(zint PROPERTIES   SOVERSION "${ZINT_VERSION_MAJOR}.${ZINT_VERSION_MINOR}"
                                            VERSION ${ZINT_VERSION})
endif()

if(ZINT_USE_PNG AND PNG_FOUND)
    zint_target_link_libraries(PNG::PNG)
    message(STATUS "Using PNG")
else()
    zint_target_compile_definitions(PRIVATE ZINT_NO_PNG)
    message(STATUS "Not using PNG")
endif()

if(ZINT_TEST)
    zint_target_compile_definitions(PUBLIC ZINT_TEST)
endif()

check_c_compiler_flag("-Wc90-c99-compat" C_COMPILER_FLAG_WC90_C99_COMPAT)
if(C_COMPILER_FLAG_WC90_C99_COMPAT)
    zint_target_compile_options(PRIVATE "-Wc90-c99-compat")
endif()

if(NOT MSVC)
    # Link with standard C math library.
    zint_target_link_libraries(m)
endif()

if(MSVC)
    if(ZINT_SHARED)
        target_compile_definitions(zint PRIVATE DLL_EXPORT)
    endif()
endif()

zint_target_include_directories(PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Adapted from old (2008) KDE "SetPaths.cmake" to use GNUInstallDirs
set(INSTALL_TARGETS_DEFAULT_ARGS RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
                                 LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
                                 ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT Devel)

if(ZINT_SHARED)
    install(TARGETS zint EXPORT zint-targets ${INSTALL_TARGETS_DEFAULT_ARGS})
endif()
if(ZINT_STATIC)
    install(TARGETS zint-static EXPORT zint-targets ${INSTALL_TARGETS_DEFAULT_ARGS})
endif()
install(EXPORT zint-targets NAMESPACE zint:: DESTINATION "${CMAKE_INSTALL_DATADIR}/zint")
install(FILES zint.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} COMPONENT Devel)

if(ZINT_TEST)
    add_subdirectory(tests)
endif()
