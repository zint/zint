# - Try to find the Zint barcode library
# Once done this will define
#
#  LIBZINT_FOUND - System has Zint barcode
#  LIBZINT_INCLUDE_DIRS - The Zint barcode include directory
#  LIBZINT_LIBRARIES - The libraries needed to use Zint barcode
#  LIBZINT_DEFINITIONS - Definitions needed to use Zint barcode
#  LIBZINT_VERSION_STRING - the version of Zint barcode found

set (LIBZINT_DEFINITIONS "")

find_path (LIBZINT_INCLUDE_DIR NAMES zint.h)

find_library (LIBZINT_LIBRARY NAMES zint )

if (LIBZINT_LIBRARY AND LIBZINT_INCLUDE_DIR)

  set (LIBZINT_INCLUDE_DIRS ${LIBZINT_INCLUDE_DIR})
  set (LIBZINT_LIBRARIES ${LIBZINT_LIBRARY})
  set (LIBZINT_DEFINITIONS "")

  if (NOT TARGET ZINT::ZINT)

    add_library (ZINT::ZINT UNKNOWN IMPORTED)
    set_target_properties (ZINT::ZINT PROPERTIES
      INTERFACE_COMPILE_DEFINITIONS     "${LIBZINT_DEFINITIONS}"
      INTERFACE_INCLUDE_DIRECTORIES     "${LIBZINT_INCLUDE_DIRS}"
      IMPORTED_LINK_INTERFACE_LANGUAGES "C"
      IMPORTED_LOCATION                 "${LIBZINT_LIBRARY}"
    )

  endif ()

endif ()

if (LIBZINT_INCLUDE_DIR AND EXISTS "${LIBZINT_INCLUDE_DIR}/zintconfig.h")
  file (STRINGS "${LIBZINT_INCLUDE_DIR}/zintconfig.h" ZINT_MAJOR_H REGEX "^#define ZINT_VERSION_MAJOR *[0-9]*")
  file (STRINGS "${LIBZINT_INCLUDE_DIR}/zintconfig.h" ZINT_MINOR_H REGEX "^#define ZINT_VERSION_MINOR *[0-9]*")
  file (STRINGS "${LIBZINT_INCLUDE_DIR}/zintconfig.h" ZINT_MICRO_H REGEX "^#define ZINT_VERSION_RELEASE *[0-9]*")
  string (REGEX REPLACE "^.*VERSION_MAJOR *([0-9]*)" "\\1" ZINT_MAJOR ${ZINT_MAJOR_H})
  string (REGEX REPLACE "^.*VERSION_MINOR *([0-9]*)" "\\1" ZINT_MINOR ${ZINT_MINOR_H})
  string (REGEX REPLACE "^.*VERSION_RELEASE *([0-9]*)" "\\1" ZINT_MICRO ${ZINT_MICRO_H})
  set (LIBZINT_VERSION_STRING ${ZINT_MAJOR}.${ZINT_MINOR}.${ZINT_MICRO})
endif()

# handle the QUIETLY and REQUIRED arguments and set LIBZINT_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibZint
                                  REQUIRED_VARS LIBZINT_LIBRARY LIBZINT_INCLUDE_DIR
                                  VERSION_VAR LIBZINT_VERSION_STRING)

mark_as_advanced(LIBZINT_INCLUDE_DIR LIBZINT_LIBRARY)
