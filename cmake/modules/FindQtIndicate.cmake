# - Try to find the indicate-qt library
# Once done this will define
#
#  QtIndicate_FOUND - system has the QtIndicate library
#  QTINDICATE_INCLUDE_DIRS - the QtIndicate include directories
#  QTINDICATE_LIBRARY - Link this to use the QtIndicate library
#
# Copyright (c) 2010, Mehrdad Momeny <mehrdad.momeny@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (NOT WIN32)
    find_package(PkgConfig)
    pkg_check_modules(PC_QTINDICATE indicate-qt)
endif(NOT WIN32)

find_library(QTINDICATE_LIBRARY NAMES indicate-qt
    HINTS  ${PC_QTINDICATE_LIBDIR} ${PC_QTINDICATE_LIBRARY_DIRS}
)

find_path(QTINDICATE_INCLUDE_DIR libindicate-qt/qindicateserver.h
    HINTS ${PC_QTINDICATE_INCLUDEDIR} ${PC_QTINDICATE_INCLUDE_DIRS}
)
set(QTINDICATE_INCLUDE_DIRS ${QTINDICATE_INCLUDE_DIR} ${QTINDICATE_INCLUDE_DIR}/libindicate-qt)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(QtIndicate DEFAULT_MSG QTINDICATE_LIBRARY QTINDICATE_INCLUDE_DIRS)

mark_as_advanced(QTINDICATE_INCLUDE_DIRS QTINDICATE_LIBRARY)

if (QTINDICATE_INCLUDE_DIRS AND QTINDICATE_LIBRARY)
    set(QtIndicate_FOUND TRUE)
endif (QTINDICATE_INCLUDE_DIRS AND QTINDICATE_LIBRARY)
