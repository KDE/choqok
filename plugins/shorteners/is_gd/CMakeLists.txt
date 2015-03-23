set(choqok_is_gd_SRCS
    is_gd.cpp
)

kconfig_add_kcfg_files(choqok_is_gd_SRCS is_gd_settings.kcfgc)

add_library(choqok_is_gd MODULE ${choqok_is_gd_SRCS})

kcoreaddons_desktop_to_json(choqok_is_gd choqok_is_gd.desktop)

target_link_libraries(choqok_is_gd
PUBLIC
    Qt5::Core
    KF5::CoreAddons
    KF5::I18n
    KF5::KIOCore
    KF5::KDELibs4Support
    choqok
)

install(TARGETS choqok_is_gd DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES choqok_is_gd.desktop DESTINATION ${SERVICES_INSTALL_DIR})

set(kcm_choqok_is_gd_PART_SRCS
    is_gd_config.cpp
)

ki18n_wrap_ui(kcm_choqok_is_gd_PART_SRCS is_gd_prefs.ui)

kconfig_add_kcfg_files(kcm_choqok_is_gd_PART_SRCS is_gd_settings.kcfgc)

add_library(kcm_choqok_is_gd MODULE ${kcm_choqok_is_gd_PART_SRCS})

kcoreaddons_desktop_to_json(kcm_choqok_is_gd choqok_is_gd_config.desktop)

target_link_libraries(kcm_choqok_is_gd
PUBLIC
    Qt5::Core
    KF5::ConfigWidgets
    KF5::CoreAddons
    KF5::I18n
    choqok
)

install(TARGETS kcm_choqok_is_gd DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES is_gd_settings.kcfg DESTINATION ${KCFG_INSTALL_DIR})
install(FILES choqok_is_gd_config.desktop DESTINATION ${SERVICES_INSTALL_DIR})
