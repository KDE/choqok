add_library(choqok_is_gd MODULE
    is_gd.cpp
)

kconfig_add_kcfg_files(choqok_is_gd is_gd_settings.kcfgc)

kcoreaddons_desktop_to_json(choqok_is_gd choqok_is_gd.desktop)

target_link_libraries(choqok_is_gd
PUBLIC
    Qt::Core
    KF5::CoreAddons
    KF5::I18n
    KF5::KIOCore
    choqok
)

install(TARGETS choqok_is_gd DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES choqok_is_gd.desktop DESTINATION ${KDE_INSTALL_KSERVICESDIR})

add_library(kcm_choqok_is_gd MODULE
    is_gd_config.cpp
)

ki18n_wrap_ui(kcm_choqok_is_gd is_gd_prefs.ui)

kconfig_add_kcfg_files(kcm_choqok_is_gd is_gd_settings.kcfgc)

kcoreaddons_desktop_to_json(kcm_choqok_is_gd choqok_is_gd_config.desktop)

target_link_libraries(kcm_choqok_is_gd
PUBLIC
    Qt::Core
    KF5::ConfigWidgets
    KF5::CoreAddons
    KF5::I18n
    choqok
)

install(TARGETS kcm_choqok_is_gd DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES is_gd_settings.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
install(FILES choqok_is_gd_config.desktop DESTINATION ${KDE_INSTALL_KSERVICESDIR})
