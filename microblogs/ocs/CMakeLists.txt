
find_package(KF5Attica)

IF(KF5Attica_FOUND)
    message(STATUS "FOUND LibAttica: Will build \"Open Collaboration Services\" plugin")
    include_directories(
    ${CHOQOK_INCLUDES}
    ${CMAKE_CURRENT_BINARY_DIR}
    )

    set( choqok_ocs_SRCS
    ocsaccount.cpp
    ocsmicroblog.cpp
    ocsconfigurewidget.cpp
    )

    ki18n_wrap_ui(choqok_ocs_SRCS
    ocsconfigurebase.ui
    )

    add_library(choqok_ocs MODULE ${choqok_ocs_SRCS})

    target_link_libraries(choqok_ocs KF5::Attica choqok)

    # Install

    install(TARGETS choqok_ocs DESTINATION ${PLUGIN_INSTALL_DIR})
    install( FILES choqok_ocs.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

    add_subdirectory(icons)
ELSE(KF5Attica_FOUND)
    message(WARNING "Optional dependency \"LibAttica\" NOT FOUND, won't build \"Open Collaboration Services\" plugin")
ENDIF(KF5Attica_FOUND)
