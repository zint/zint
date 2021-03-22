# (c) 2008 by BogDan Vatra < bogdan@licentia.eu >
# vim: set ts=4 sw=4 et :

project(zint)

configure_file(zintconfig.h.in ../../backend/zintconfig.h)

find_package(PNG)

set(zint_COMMON_SRCS common.c library.c large.c reedsol.c gs1.c eci.c general_field.c sjis.c gb2312.c gb18030.c)
set(zint_ONEDIM_SRCS code.c code128.c 2of5.c upcean.c telepen.c medical.c plessey.c rss.c)
set(zint_POSTAL_SRCS postal.c auspost.c imail.c mailmark.c)
set(zint_TWODIM_SRCS code16k.c codablock.c dmatrix.c pdf417.c qr.c maxicode.c composite.c aztec.c code49.c code1.c gridmtx.c hanxin.c dotcode.c ultra.c)
set(zint_OUTPUT_SRCS vector.c ps.c svg.c emf.c bmp.c pcx.c gif.c png.c tif.c raster.c output.c)
set(zint_SRCS ${zint_OUTPUT_SRCS} ${zint_COMMON_SRCS} ${zint_ONEDIM_SRCS} ${zint_POSTAL_SRCS} ${zint_TWODIM_SRCS})

add_library(zint SHARED ${zint_SRCS})

if(ZINT_STATIC)
    add_library(zint-static STATIC ${zint_SRCS})
endif()

set_target_properties(zint PROPERTIES   SOVERSION "${ZINT_VERSION_MAJOR}.${ZINT_VERSION_MINOR}"
                                        VERSION ${ZINT_VERSION})

if(PNG_FOUND)
    target_link_libraries(zint PNG::PNG)
else()
    target_compile_definitions(zint PUBLIC NO_PNG)
endif()

if(ZINT_TEST)
    target_compile_definitions(zint PUBLIC ZINT_TEST)
endif()

if(NOT MSVC)
    # Link with standard C math library.
    target_link_libraries(zint m)
endif()

if(MSVC)
    # "BUILD_SHARED_LIBS" is a CMake defined variable, see documentation.
    if(BUILD_SHARED_LIBS)
        target_compile_definitions(zint PRIVATE DLL_EXPORT)
    endif()
endif()

install(TARGETS zint  ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES zint.h DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)

if(ZINT_TEST)
    add_subdirectory(tests)
endif(ZINT_TEST)
