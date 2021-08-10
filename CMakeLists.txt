# Copyright (C) 2008 by BogDan Vatra < bogdan@licentia.eu >
# Copyright (C) 2009-2021 Robin Stuart <rstuart114@gmail.com>
# vim: set ts=4 sw=4 et :

cmake_minimum_required(VERSION 3.5)
project(zint-package)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(ZINT_VERSION_MAJOR 2)
set(ZINT_VERSION_MINOR 10)
set(ZINT_VERSION_RELEASE 0)
set(ZINT_VERSION_BUILD 0) # Set to 0 before release, set to 9 after release
set(ZINT_VERSION "${ZINT_VERSION_MAJOR}.${ZINT_VERSION_MINOR}.${ZINT_VERSION_RELEASE}.${ZINT_VERSION_BUILD}")

add_definitions(-DZINT_VERSION=\"${ZINT_VERSION}\")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

option(ZINT_DEBUG    "Set debug compile flags"         OFF)
option(ZINT_SANITIZE "Set sanitize compile/link flags" OFF)
option(ZINT_TEST     "Set test compile flag"           OFF)
option(ZINT_COVERAGE "Set code coverage flags"         OFF)
option(ZINT_STATIC   "Build static library"            OFF)
option(ZINT_USE_PNG  "Build with PNG support"          ON)
option(ZINT_USE_QT   "Build with QT support"           ON)

include(SetPaths.cmake)

include(CheckCXXCompilerFlag)
include(CheckFunctionExists)

if(NOT MSVC) # Use default warnings if MSVC otherwise inundated
    check_cxx_compiler_flag("-Wall" CXX_COMPILER_FLAG_WALL)
    if(CXX_COMPILER_FLAG_WALL)
        add_compile_options("-Wall")
    endif()

    check_cxx_compiler_flag("-Wextra" CXX_COMPILER_FLAG_WEXTRA)
    if(CXX_COMPILER_FLAG_WEXTRA)
        add_compile_options("-Wextra")
    endif()

    check_cxx_compiler_flag("-Wpedantic" CXX_COMPILER_FLAG_WPEDANTIC)
    if(CXX_COMPILER_FLAG_WPEDANTIC)
        add_compile_options("-Wpedantic")
    endif()
endif()

if(ZINT_DEBUG)
    check_cxx_compiler_flag("-g" CXX_COMPILER_FLAG_G)
    if(CXX_COMPILER_FLAG_G)
        add_compile_options("-g")
    endif()

    check_cxx_compiler_flag("-O0" CXX_COMPILER_FLAG_O0)
    if(CXX_COMPILER_FLAG_O0)
        add_compile_options("-O0")
    endif()
endif()

if(ZINT_TEST)
    enable_testing()
endif()

if(ZINT_COVERAGE)
    set(CMAKE_REQUIRED_LIBRARIES -fprofile-arcs)
    check_cxx_compiler_flag(--coverage CXX_COMPILER_FLAG_COVERAGE)
    unset(CMAKE_REQUIRED_LIBRARIES)
    if(CXX_COMPILER_FLAG_COVERAGE)
        add_compile_options(--coverage)
        link_libraries(-fprofile-arcs)

        check_cxx_compiler_flag(-O0 CXX_COMPILER_FLAG_O0)
        if(CXX_COMPILER_FLAG_O0)
            add_compile_options(-O0)
        endif()
    endif()
endif()

if(ZINT_SANITIZE)
    if(MSVC)
        if(MSVC_VERSION GREATER_EQUAL 1920)
            add_compile_options(-fsanitize=address)
            message(STATUS "ZINT_SANITIZE: setting -fsanitize=address for MSVC 2019")
        else()
            message(STATUS "ZINT_SANITIZE: ignoring for MSVC < 2019")
        endif()
    else()
        set(SANITIZERS address undefined)
        foreach(sanitizer IN ITEMS ${SANITIZERS})
            set(CMAKE_REQUIRED_LIBRARIES -fsanitize=${sanitizer})
            check_cxx_compiler_flag(-fsanitize=${sanitizer} CXX_COMPILER_FLAG_FSANITIZE_${sanitizer})
            if(CXX_COMPILER_FLAG_FSANITIZE_${sanitizer})
                add_compile_options(-fsanitize=${sanitizer})
                link_libraries(-fsanitize=${sanitizer})
            endif()
            unset(CMAKE_REQUIRED_LIBRARIES)
        endforeach()

        if(NOT ZINT_DEBUG AND CMAKE_CXX_COMPILER_ID MATCHES "GNU")
            # Gives warning on MainWindow::setupUI() and retries (& takes forever) if var-tracking-assignments enabled
            add_compile_options(-fno-var-tracking-assignments)
        endif()
    endif()
endif()

if(APPLE)
    if(UNIVERSAL) # TODO: make universal binary
        if(NOT ZINT_HAS_BEEN_RUN_BEFORE)
            if(EXISTS /Developer/SDKs/MacOSX10.5.sdk OR EXISTS /SDKs/MacOSX10.5.sdk)
                set(CMAKE_OSX_ARCHITECTURES "ppc;i386;ppc64;x86_64" CACHE STRING "Build architectures for OSX" FORCE)
                set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility-inlines-hidden -Wl -single_module " CACHE STRING
                    "Flags used by the compiler during all build types." FORCE)
            elseif(EXISTS /Developer/SDKs/MacOSX10.4u.sdk OR EXISTS /SDKs/MacOSX10.4u.sdk)
                set(CMAKE_OSX_ARCHITECTURES "ppc;i386" CACHE STRING "Build architectures for OSX" FORCE)
                set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility-inlines-hidden -Wl -single_module " CACHE STRING
                    "Flags used by the compiler during all build types." FORCE)
            endif()
            message("Build architectures for OSX:${CMAKE_OSX_ARCHITECTURES}")
        endif()
    else()
        set(CMAKE_OSX_SYSROOT "/")
    endif()
endif()

check_function_exists(getopt HAVE_GETOPT)
if(NOT HAVE_GETOPT)
    add_subdirectory(getopt)
endif()

add_subdirectory(backend)
add_subdirectory(frontend)

if(NOT ZINT_USE_QT)
    message(STATUS "Qt support was disabled for this build")
elseif($ENV{CMAKE_PREFIX_PATH} MATCHES "6[.][0-9][.][0-9]")
    set(USE_QT6 TRUE)
    message(STATUS "Using Qt6")
    cmake_policy(SET CMP0012 NEW) # Recognize constants in if()
    cmake_policy(SET CMP0072 NEW) # Choose OpenGL over legacy GL
    find_package(Qt6Widgets)
    find_package(Qt6Gui)
    find_package(Qt6UiTools)
    find_package(Qt6Xml)

    if(Qt6Widgets_FOUND AND Qt6Gui_FOUND AND Qt6UiTools_FOUND AND Qt6Xml_FOUND)
        message(STATUS "Qt version: " ${Qt6Core_VERSION_MAJOR}.${Qt6Core_VERSION_MINOR}.${Qt6Core_VERSION_PATCH})
        add_subdirectory(backend_qt)
        add_subdirectory(frontend_qt)
    else()
        message(STATUS "Could NOT find Qt6")
    endif()
else()
    message(STATUS "Using Qt5")
    find_package(Qt5Widgets)
    find_package(Qt5Gui)
    find_package(Qt5UiTools)
    find_package(Qt5Xml)

    if(Qt5Widgets_FOUND AND Qt5Gui_FOUND AND Qt5UiTools_FOUND AND Qt5Xml_FOUND)
        message(STATUS "Qt version: " ${Qt5Core_VERSION_STRING})
        # Old Qt does not provide QT_VERSION_MAJOR
        if (NOT QT_VERSION_MAJOR)
            string(SUBSTRING ${Qt5Core_VERSION_STRING} 0 1 QT_VERSION_MAJOR)
        endif()
        add_subdirectory(backend_qt)
        add_subdirectory(frontend_qt)
    else()
        message(STATUS "Could NOT find Qt5")
    endif()
endif()

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)

add_custom_target(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

# staniek: don't install
if(DATA_INSTALL_DIR)
    set(CMAKE_MODULES_INSTALL_PATH ${DATA_INSTALL_DIR}/cmake/modules)
else()
    set(CMAKE_MODULES_INSTALL_PATH ${CMAKE_ROOT}/Modules)
endif()

install(FILES cmake/modules/FindZint.cmake DESTINATION ${CMAKE_MODULES_INSTALL_PATH} COMPONENT Devel)

# This needs to be run very last so other parts of the scripts can take
# advantage of this.
if(NOT ZINT_HAS_BEEN_RUN_BEFORE)
    set(ZINT_HAS_BEEN_RUN_BEFORE 1 CACHE INTERNAL
        "Flag to track whether this is the first time running CMake or if CMake has been configured before")
endif()
