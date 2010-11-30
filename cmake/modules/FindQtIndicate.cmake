# - Try to find the indicate-qt library
# Once done this will define
#
#  QTINDICATE_FOUND - system has the QtIndicate library
#  QTINDICATE_INCLUDE_DIR - the QtIndicate include directory
#  QTINDICATE_LIBRARY - Link this to use the QtIndicate library
#
# Copyright (c) 2010, Mehrdad Momeny <mehrdad.momeny@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (QTINDICATE_INCLUDE_DIR AND QTINDICATE_LIBRARY)
    # in cache already
set(QTINDICATE_FOUND TRUE)

else (QTINDICATE_INCLUDE_DIR AND QTINDICATE_LIBRARY)
    if (NOT WIN32)
        find_package(PkgConfig)
        pkg_check_modules(PC_QTINDICATE indicate-qt>=0.2.1)
    endif(NOT WIN32)

    find_path(QTINDICATE_INCLUDE_DIR libindicate-qt/qindicateserver.h
    HINTS
    ${PC_QTINDICATE_INCLUDE_DIRS}
    )

    find_library(QTINDICATE_LIBRARY NAMES libindicate-qt
    HINTS
    ${PC_QTINDICATE_LIBRARY_DIRS}
    )

    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(QtIndicate DEFAULT_MSG QTINDICATE_LIBRARY QTINDICATE_INCLUDE_DIR)

    mark_as_advanced(QTINDICATE_INCLUDE_DIR QTINDICATE_LIBRARY)

endif (QTINDICATE_INCLUDE_DIR AND QTINDICATE_LIBRARY)
