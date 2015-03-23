set(choqok_flickr_PART_SRCS
    flickr.cpp
)

kconfig_add_kcfg_files(choqok_flickr_PART_SRCS flickrsettings.kcfgc)

add_library(choqok_flickr MODULE ${choqok_flickr_PART_SRCS})

kcoreaddons_desktop_to_json(choqok_flickr choqok_flickr.desktop)

target_link_libraries(choqok_flickr
PUBLIC
    Qt5::Core
    Qt5::Xml
    KF5::ConfigWidgets
    KF5::CoreAddons
    KF5::KIOCore
    KF5::KDELibs4Support
    choqok
)

install(TARGETS choqok_flickr DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES choqok_flickr.desktop DESTINATION ${SERVICES_INSTALL_DIR})

set(kcm_choqok_flickr_PART_SRCS
    flickrconfig.cpp
)

ki18n_wrap_ui(kcm_choqok_flickr_PART_SRCS flickrprefs.ui)

kconfig_add_kcfg_files(kcm_choqok_flickr_PART_SRCS flickrsettings.kcfgc)

add_library(kcm_choqok_flickr MODULE ${kcm_choqok_flickr_PART_SRCS})

kcoreaddons_desktop_to_json(kcm_choqok_flickr choqok_flickr_config.desktop)

target_link_libraries(kcm_choqok_flickr
PUBLIC
    Qt5::Core
    Qt5::Widgets
    Qt5::Xml
    KF5::ConfigCore
    KF5::CoreAddons
    KF5::I18n
    KF5::KIOCore
    KF5::WidgetsAddons
    KF5::KDELibs4Support
    choqok
)

install(TARGETS kcm_choqok_flickr DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES flickrsettings.kcfg DESTINATION ${KCFG_INSTALL_DIR})
install(FILES choqok_flickr_config.desktop DESTINATION ${SERVICES_INSTALL_DIR})

add_subdirectory(icons)
