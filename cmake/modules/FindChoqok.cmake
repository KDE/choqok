# - Try to find the Choqok library
# Once done this will define
#
#  Choqok_FOUND - system has choqok
#  CHOQOK_INCLUDE_DIR - the choqok include directory
#  CHOQOK_LIBRARIES - Link these to use choqok

# Copyright (c) 2010 Mehrdad Momeny <mehrdad.momeny@gmail.com>
# I've wrote this based on FindKopete.cmake
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(CHOQOK_INCLUDE_DIR AND CHOQOK_LIBRARIES)

  # read from cache
  set(Choqok_FOUND TRUE)

else(CHOQOK_INCLUDE_DIR AND CHOQOK_LIBRARIES)

  FIND_PATH(CHOQOK_INCLUDE_DIR
    NAMES
    choqok/choqok_export.h
    PATHS 
    ${KDE4_INCLUDE_DIR}
    ${INCLUDE_INSTALL_DIR}
    )
  
  FIND_LIBRARY(CHOQOK_LIBRARIES
    NAMES
    choqok
    PATHS
    ${KDE4_LIB_DIR}
    ${LIB_INSTALL_DIR}
    )
  if(CHOQOK_INCLUDE_DIR AND CHOQOK_LIBRARIES)
    set(Choqok_FOUND TRUE)
  endif(CHOQOK_INCLUDE_DIR AND CHOQOK_LIBRARIES)

  if(Choqok_FOUND)
    if(NOT Choqok_FIND_QUIETLY)
      message(STATUS "Found Choqok: ${CHOQOK_LIBRARIES}")
    endif(NOT Choqok_FIND_QUIETLY)
  else(Choqok_FOUND)
    if(Choqok_FIND_REQUIRED)
      if(NOT CHOQOK_INCLUDE_DIR)
	message(FATAL_ERROR "Could not find Choqok includes.")
      endif(NOT CHOQOK_INCLUDE_DIR)
      if(NOT CHOQOK_LIBRARIES)
	message(FATAL_ERROR "Could not find Choqok library.")
      endif(NOT CHOQOK_LIBRARIES)
    else(Choqok_FIND_REQUIRED)
      if(NOT CHOQOK_INCLUDE_DIR)
        message(STATUS "Could not find Choqok includes.")
      endif(NOT CHOQOK_INCLUDE_DIR)
      if(NOT CHOQOK_LIBRARIES)
        message(STATUS "Could not find Choqok library.")
      endif(NOT CHOQOK_LIBRARIES)
    endif(Choqok_FIND_REQUIRED)
  endif(Choqok_FOUND)

endif(CHOQOK_INCLUDE_DIR AND CHOQOK_LIBRARIES)
