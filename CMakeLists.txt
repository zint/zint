# (c) 2008 by BogDan Vatra < bogdan@licentia.eu >
# vim: set ts=4 sw=4 et :

cmake_minimum_required(VERSION 3.5)
project(zint-package)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

#set(CMAKE_VERBOSE_MAKEFILE ON)
#comment or remove the above line before release

set(ZINT_VERSION_MAJOR 2)
set(ZINT_VERSION_MINOR 9)
set(ZINT_VERSION_RELEASE 1)
set(ZINT_VERSION_BUILD 9) # Set to 0 before release, set to 9 after release
set(ZINT_VERSION "${ZINT_VERSION_MAJOR}.${ZINT_VERSION_MINOR}.${ZINT_VERSION_RELEASE}.${ZINT_VERSION_BUILD}")

add_definitions(-DZINT_VERSION=\"${ZINT_VERSION}\")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

option(ZINT_DEBUG    "Set debug compile flag"          OFF)
option(ZINT_SANITIZE "Set sanitize compile/link flags" OFF)
option(ZINT_TEST     "Set test compile flag"           OFF)
option(ZINT_STATIC   "Build static library"            OFF)
option(ZINT_USE_PNG  "Build with PNG support"          ON)
option(ZINT_USE_QT   "Build with QT support"           ON)

include(SetPaths.cmake)

include(CheckCXXCompilerFlag)

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

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUCC)
    if(ZINT_SANITIZE)
        add_compile_options(-fsanitize=undefined -fsanitize=address)
        link_libraries(-fsanitize=undefined -fsanitize=address)
        # Gives warning on MainWindow::setupUI() and retries (and takes forever) if var-tracking-assignments enabled
        add_compile_options(-fno-var-tracking-assignments)
    endif()
endif()

IF(APPLE)
    IF (UNIVERSAL) # TODO: make universal binary
        IF(NOT ZINT_HAS_BEEN_RUN_BEFORE)
            IF(EXISTS /Developer/SDKs/MacOSX10.5.sdk OR EXISTS /SDKs/MacOSX10.5.sdk)
                SET(CMAKE_OSX_ARCHITECTURES "ppc;i386;ppc64;x86_64" CACHE STRING "Build architectures for OSX" FORCE)
                SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility-inlines-hidden -Wl -single_module " CACHE STRING "Flags used by the compiler during all build types." FORCE)
            ELSE(EXISTS /Developer/SDKs/MacOSX10.5.sdk OR EXISTS /SDKs/MacOSX10.5.sdk)
                IF(EXISTS /Developer/SDKs/MacOSX10.4u.sdk OR EXISTS /SDKs/MacOSX10.4u.sdk)
                    SET(CMAKE_OSX_ARCHITECTURES "ppc;i386" CACHE STRING "Build architectures for OSX" FORCE)
                    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility-inlines-hidden -Wl -single_module " CACHE STRING "Flags used by the compiler during all build types." FORCE)
               ENDIF(EXISTS /Developer/SDKs/MacOSX10.4u.sdk OR EXISTS /SDKs/MacOSX10.4u.sdk)
            ENDIF(EXISTS /Developer/SDKs/MacOSX10.5.sdk OR EXISTS /SDKs/MacOSX10.5.sdk)
            message("Build architectures for OSX:${CMAKE_OSX_ARCHITECTURES}")
        ENDIF(NOT ZINT_HAS_BEEN_RUN_BEFORE)

    ELSE (UNIVERSAL)
        SET(CMAKE_OSX_SYSROOT "/")
    ENDIF (UNIVERSAL)
ENDIF(APPLE)

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
