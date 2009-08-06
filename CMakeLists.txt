project(choqok)
cmake_minimum_required(VERSION 2.6)
find_package(KDE4 REQUIRED)
include (KDE4Defaults)
add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
set(CHOQOK_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(CHOQOK_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

set(CHOQOK_INCLUDES
${KDE4_INCLUDES}
${QT_INCLUDES}
${CHOQOK_SOURCE_DIR}/choqok/
${CHOQOK_BINARY_DIR}}/choqok/
)


add_subdirectory( choqok )
add_subdirectory( icons )
