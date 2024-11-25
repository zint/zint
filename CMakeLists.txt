# Copyright (C) 2008 by BogDan Vatra < bogdan@licentia.eu >
# Copyright (C) 2009-2024 Robin Stuart <rstuart114@gmail.com>
# vim: set ts=4 sw=4 et :

cmake_minimum_required(VERSION 3.10)
project(zint-package)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(ZINT_VERSION_MAJOR 2)
set(ZINT_VERSION_MINOR 13)
set(ZINT_VERSION_RELEASE 0)
set(ZINT_VERSION_BUILD 9) # Set to 0 before release, set to 9 after release
set(ZINT_VERSION "${ZINT_VERSION_MAJOR}.${ZINT_VERSION_MINOR}.${ZINT_VERSION_RELEASE}.${ZINT_VERSION_BUILD}")

add_definitions(-DZINT_VERSION=\"${ZINT_VERSION}\")

option(ZINT_DEBUG     "Set debug compile flags"         OFF)
option(ZINT_NOOPT     "Set no optimize compile flags"   OFF)
option(ZINT_SANITIZE  "Set sanitize compile/link flags" OFF)
option(ZINT_TEST      "Set test compile flag"           OFF)
option(ZINT_COVERAGE  "Set code coverage flags"         OFF)
option(ZINT_SHARED    "Build shared library"            ON)
option(ZINT_STATIC    "Build static library"            OFF)
option(ZINT_USE_PNG   "Build with PNG support"          ON)
option(ZINT_USE_QT    "Build with Qt support"           ON)
option(ZINT_QT6       "If ZINT_USE_QT, use Qt6"         OFF)
option(ZINT_UNINSTALL "Add uninstall target"            ON)
option(ZINT_FRONTEND  "Build frontend"                  ON)

if(NOT ZINT_SHARED AND NOT ZINT_STATIC)
    message(SEND_ERROR "Either ZINT_SHARED or ZINT_STATIC or both must be set")
endif()

include(GNUInstallDirs)
# Taken from old (2008) KDE "SetPaths.cmake"
# Set a default build type for single-configuration CMake generators if no build type is set
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

include(CheckCCompilerFlag)
include(CheckFunctionExists)

if(NOT MSVC) # Use default warnings if MSVC otherwise inundated
    check_c_compiler_flag("-Wall" C_COMPILER_FLAG_WALL)
    if(C_COMPILER_FLAG_WALL)
        add_compile_options("-Wall")
    endif()

    check_c_compiler_flag("-Wextra" C_COMPILER_FLAG_WEXTRA)
    if(C_COMPILER_FLAG_WEXTRA)
        add_compile_options("-Wextra")
    endif()

    # gcc complains about "%n$" `errtxtf()` arguments if "-Wpedantic" used, so only use for clang
    if(CMAKE_C_COMPILER_ID MATCHES "Clang")
        check_c_compiler_flag("-Wpedantic" C_COMPILER_FLAG_WPEDANTIC)
        if(C_COMPILER_FLAG_WPEDANTIC)
            add_compile_options("-Wpedantic")
        endif()
    endif()

    check_c_compiler_flag("-Wundef" C_COMPILER_FLAG_WUNDEF)
    if(C_COMPILER_FLAG_WUNDEF)
        add_compile_options("-Wundef")
    endif()
endif()

if(ZINT_DEBUG)
    check_c_compiler_flag("-g" C_COMPILER_FLAG_G)
    if(C_COMPILER_FLAG_G)
        add_compile_options("-g")
    endif()
endif()

if(ZINT_NOOPT)
    check_c_compiler_flag("-O0" C_COMPILER_FLAG_O0)
    if(C_COMPILER_FLAG_O0)
        add_compile_options("-O0")
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
            check_c_compiler_flag(-fsanitize=${sanitizer} C_COMPILER_FLAG_FSANITIZE_${sanitizer})
            if(C_COMPILER_FLAG_FSANITIZE_${sanitizer})
                add_compile_options(-fsanitize=${sanitizer})
                link_libraries(-fsanitize=${sanitizer})
            endif()
            unset(CMAKE_REQUIRED_LIBRARIES)
        endforeach()

        if(NOT ZINT_DEBUG AND CMAKE_C_COMPILER_ID MATCHES "GNU")
            # Gives warning on MainWindow::setupUI() and retries (& takes forever) if var-tracking-assignments enabled
            add_compile_options(-fno-var-tracking-assignments)
        endif()
        if(CMAKE_C_COMPILER_ID MATCHES "Clang")
            # Recent clangs added deprecation warnings for `sprintf()` that are only triggered on sanitize - suppress
            add_compile_options(-Wno-deprecated-declarations)
        endif()
    endif()
endif()

if(ZINT_TEST)
    enable_testing()
endif()

if(ZINT_COVERAGE)
    set(CMAKE_REQUIRED_LIBRARIES -fprofile-arcs)
    check_c_compiler_flag(--coverage C_COMPILER_FLAG_COVERAGE)
    unset(CMAKE_REQUIRED_LIBRARIES)
    if(C_COMPILER_FLAG_COVERAGE)
        add_compile_options(--coverage)
        link_libraries(-fprofile-arcs)

        check_c_compiler_flag(-O0 C_COMPILER_FLAG_O0)
        if(C_COMPILER_FLAG_O0)
            add_compile_options(-O0)
        endif()
    endif()
endif()

check_function_exists(getopt_long_only HAVE_GETOPT_LONG_ONLY)
if(NOT HAVE_GETOPT_LONG_ONLY)
    add_subdirectory(getopt)
endif()

add_subdirectory(backend)
if(ZINT_FRONTEND)
    add_subdirectory(frontend)
endif()

if(NOT ZINT_USE_QT)
    message(STATUS "Qt support was disabled for this build")
elseif(ZINT_QT6 OR ($ENV{CMAKE_PREFIX_PATH} MATCHES "6[.][0-9][.][0-9]"))
    set(USE_QT6 TRUE)
    message(STATUS "Using Qt6")
    cmake_policy(SET CMP0012 NEW) # Recognize constants in if()
    cmake_policy(SET CMP0072 NEW) # Choose OpenGL over legacy GL
    find_package(Qt6Widgets)
    find_package(Qt6Gui)
    find_package(Qt6UiTools)
    find_package(Qt6Svg)

    if(Qt6Widgets_FOUND AND Qt6Gui_FOUND AND Qt6UiTools_FOUND AND Qt6Svg_FOUND)
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
    find_package(Qt5Svg)

    if(Qt5Widgets_FOUND AND Qt5Gui_FOUND AND Qt5UiTools_FOUND AND Qt5Svg_FOUND)
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

if(ZINT_UNINSTALL)
    configure_file(
      "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
      "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
      IMMEDIATE @ONLY)
    
    add_custom_target(uninstall
      "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")
endif()

configure_file("zint-config.cmake.in" "zint-config.cmake" @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/zint-config.cmake" DESTINATION "${CMAKE_INSTALL_DATADIR}/zint")
