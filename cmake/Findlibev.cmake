find_library(LIBEV_LIBRARY libev.so)

find_path(LIBEV_INCLUDE_DIR ev.h)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(libev DEFAULT_MSG
    LIBEV_LIBRARY LIBEV_INCLUDE_DIR)

mark_as_advanced(LIBEV_INCLUDE_DIR LIBEV_LIBRARY)

set(libev_LIBRARIES ${LIBEV_LIBRARY} )
set(libev_INCLUDE_DIRS ${LIBEV_INCLUDE_DIR} )
