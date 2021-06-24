# - Find Zint and QZint
# Find the native Zint and QZint includes and library
#
#  ZINT_INCLUDE_DIR - where to find zint.h, etc.
#  ZINT_LIBRARIES   - List of libraries when using zint.
#  ZINT_FOUND       - True if zint found.
#  QZINT_INCLUDE_DIR - where to find qzint.h, etc.
#  QZINT_LIBRARIES   - List of libraries when using qzint.
#  QZINT_FOUND       - True if qzint found.

################### FIND ZINT ######################

IF (ZINT_INCLUDE_DIR)
  # Already in cache, be silent
  SET(ZINT_FIND_QUIETLY TRUE)
ENDIF (ZINT_INCLUDE_DIR)

FIND_PATH(ZINT_INCLUDE_DIR zint.h)

FIND_LIBRARY(ZINT_LIBRARY NAMES zint )

# handle the QUIETLY and REQUIRED arguments and set ZINT_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Zint DEFAULT_MSG ZINT_LIBRARY ZINT_INCLUDE_DIR)

IF(ZINT_FOUND)
  SET( ZINT_LIBRARIES ${ZINT_LIBRARY} )
ELSE(ZINT_FOUND)
  SET( ZINT_LIBRARIES )
ENDIF(ZINT_FOUND)

MARK_AS_ADVANCED( ZINT_LIBRARY ZINT_INCLUDE_DIR )

################### FIND QZINT ######################

IF (QZINT_INCLUDE_DIR)
  # Already in cache, be silent
  SET(QZINT_FIND_QUIETLY TRUE)
ENDIF (QZINT_INCLUDE_DIR)

FIND_PATH(QZINT_INCLUDE_DIR qzint.h)

FIND_LIBRARY(QZINT_LIBRARY NAMES QZint )

# handle the QUIETLY and REQUIRED arguments and set QZINT_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(QZint DEFAULT_MSG QZINT_LIBRARY QZINT_INCLUDE_DIR)

IF(QZINT_FOUND)
  SET( QZINT_LIBRARIES ${QZINT_LIBRARY} )
ELSE(QZINT_FOUND)
  SET( QZINT_LIBRARIES )
ENDIF(QZINT_FOUND)

MARK_AS_ADVANCED( QZINT_LIBRARY QZINT_INCLUDE_DIR )
