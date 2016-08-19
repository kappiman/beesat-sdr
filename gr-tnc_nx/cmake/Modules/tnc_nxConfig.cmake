INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(PC_TNC_NX tnc_nx)

FIND_PATH(
    TNC_NX_INCLUDE_DIRS
    NAMES tnc_nx/api.h
    HINTS $ENV{TNC_NX_DIR}/include
        ${PC_TNC_NX_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/include
          /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    TNC_NX_LIBRARIES
    NAMES gnuradio-tnc_nx
    HINTS $ENV{TNC_NX_DIR}/lib
        ${PC_TNC_NX_LIBDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/lib64
          /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(TNC_NX DEFAULT_MSG TNC_NX_LIBRARIES TNC_NX_INCLUDE_DIRS)
MARK_AS_ADVANCED(TNC_NX_LIBRARIES TNC_NX_INCLUDE_DIRS)

