
find_package(LibAttica)

IF(LIBATTICA_FOUND)
    message(STATUS "FOUND LibAttica: Will build OCS plugin")
    include_directories(
    ${CHOQOK_INCLUDES}
    ${LIBATTICA_INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    )

    set( choqok_ocs_SRCS
    ocsaccount.cpp
    ocsmicroblog.cpp
    ocsconfigurewidget.cpp
    )

    kde4_add_ui_files(choqok_ocs_SRCS
    ocsconfigurebase.ui
    )

    kde4_add_plugin(choqok_ocs ${choqok_ocs_SRCS})

    target_link_libraries(choqok_ocs ${KDE4_KDEUI_LIBS} ${LIBATTICA_LIBRARIES} choqok)

    # Install

    install(TARGETS choqok_ocs DESTINATION ${PLUGIN_INSTALL_DIR})
    install( FILES choqok_ocs.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

#     add_subdirectory(icons)
ELSE(LIBATTICA_FOUND)
    message(WARNING "LibAttica NOT FOUND, won't build OCS plugin")
ENDIF(LIBATTICA_FOUND)
