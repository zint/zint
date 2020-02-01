# Copyright (C) 2019 Robin Stuart <rstuart114@gmail.com>
# Adapted from qrencode/tests/CMakeLists.txt
# Copyright (C) 2006-2017 Kentaro Fukuchi <kentaro@fukuchi.org>

cmake_minimum_required (VERSION 3.9)

enable_testing()

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

set(ZINT_DEBUG FALSE CACHE BOOL "Set debug compile flag")
set(ZINT_SANITIZE FALSE CACHE BOOL "Set sanitize compile/link flags")
set(ZINT_TEST FALSE CACHE BOOL "Set test compile flag")

find_package (LibZint 2.7.1 REQUIRED)
find_package(PNG)

if (PNG_FOUND)
    include_directories(${PNG_INCLUDES})
else (PNG_FOUND)
    add_definitions(-DNO_PNG)
endif (PNG_FOUND)

if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    if (ZINT_DEBUG)
        add_compile_options("-g")
    endif (ZINT_DEBUG)
    if (ZINT_SANITIZE)
        add_compile_options("-fsanitize=undefined")
        add_compile_options("-fsanitize=address")
        set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=undefined -fsanitize=address")
    endif (ZINT_SANITIZE)
    if (ZINT_TEST)
        add_definitions("-DZINT_TEST")
    endif (ZINT_TEST)
endif ()

add_library(testcommon
    testcommon.c testcommon.h)
if (PNG_FOUND)
    target_link_libraries(testcommon ZINT::ZINT ${PNG_LIBRARIES})
else (PNG_FOUND)
    target_link_libraries(testcommon ZINT::ZINT)
endif (PNG_FOUND)

macro(zint_add_test test_name test_command)
    set(ADDITIONAL_LIBS "${ARGN}" ${LIBRARY_FLAGS})
    add_executable(${test_command} ${test_command}.c)
    target_link_libraries(${test_command} testcommon ${ADDITIONAL_LIBS})
    add_test(${test_name} ${test_command})
endmacro(zint_add_test)

zint_add_test(channel, test_channel)
zint_add_test(common, test_common)
zint_add_test(composite, test_composite)
zint_add_test(dmatrix, test_dmatrix)
zint_add_test(eci, test_eci)
zint_add_test(gb18030, test_gb18030)
zint_add_test(gb2312, test_gb2312)
zint_add_test(gridmtx, test_gridmtx)
zint_add_test(gs1, test_gs1)
zint_add_test(hanxin, test_hanxin)
zint_add_test(imail, test_imail)
zint_add_test(library, test_library)
zint_add_test(mailmark, test_mailmark)
zint_add_test(maxicode, test_maxicode)
zint_add_test(pdf417, test_pdf417)
zint_add_test(postal, test_postal)
zint_add_test(qr, test_qr)
zint_add_test(raster, test_raster)
zint_add_test(rss, test_rss)
zint_add_test(sjis, test_sjis)
zint_add_test(upcean, test_upcean)
zint_add_test(vector, test_vector)
