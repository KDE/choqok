include_directories(
    ${CHOQOK_INCLUDES}
)

set(choqok_longurl_PART_SRCS
    longurl.cpp
)

# kconfig_add_kcfg_files(choqok_longurl_PART_SRCS longurlsettings.kcfgc)

add_library(choqok_longurl MODULE ${choqok_longurl_PART_SRCS})

kcoreaddons_desktop_to_json(choqok_longurl choqok_longurl.desktop)

target_link_libraries(choqok_longurl
PUBLIC
    Qt5::Core
    KF5::CoreAddons
    KF5::KIOCore
    KF5::KIOWidgets
    choqok
)

install(TARGETS choqok_longurl DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES choqok_longurl.desktop DESTINATION ${SERVICES_INSTALL_DIR})
