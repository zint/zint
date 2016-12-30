# (c) 2008 by BogDan Vatra < bogdan@licentia.eu >

project(zint)

find_package(PNG)

set(zint_COMMON_SRCS common.c library.c render.c large.c reedsol.c gs1.c eci.c)
set(zint_ONEDIM_SRCS code.c code128.c 2of5.c upcean.c telepen.c medical.c plessey.c rss.c)
set(zint_POSTAL_SRCS postal.c auspost.c imail.c)
set(zint_TWODIM_SRCS code16k.c codablock.c dmatrix.c pdf417.c qr.c maxicode.c composite.c aztec.c code49.c code1.c gridmtx.c hanxin.c dotcode.c)
set(zint_OUTPUT_SRCS render.c ps.c svg.c emf.c bmp.c pcx.c gif.c png.c tif.c raster.c)
set(zint_SRCS ${zint_OUTPUT_SRCS} ${zint_COMMON_SRCS} ${zint_ONEDIM_SRCS} ${zint_POSTAL_SRCS} ${zint_TWODIM_SRCS})

if(PNG_FOUND)
    include_directories( ${PNG_INCLUDES} )
else(PNG_FOUND)
    add_definitions (-DNO_PNG)
endif(PNG_FOUND)

add_library(zint SHARED ${zint_SRCS})

set_target_properties(zint PROPERTIES 	SOVERSION "${ZINT_VERSION_MAJOR}.${ZINT_VERSION_MINOR}"
					VERSION ${ZINT_VERSION})

if(PNG_FOUND)
    target_link_libraries(zint ${PNG_LIBRARIES} )
endif(PNG_FOUND)
target_link_libraries(zint -lm)

install(TARGETS zint  ${INSTALL_TARGETS_DEFAULT_ARGS} )
install(FILES zint.h DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)
