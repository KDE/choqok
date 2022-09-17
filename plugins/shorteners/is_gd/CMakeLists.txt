kcoreaddons_add_plugin(choqok_is_gd SOURCES is_gd.cpp INSTALL_NAMESPACE "choqok_plugins")

kconfig_add_kcfg_files(choqok_is_gd is_gd_settings.kcfgc)

target_link_libraries(choqok_is_gd
PUBLIC
    Qt::Core
    KF5::CoreAddons
    KF5::I18n
    KF5::KIOCore
    choqok
)
kcoreaddons_add_plugin(kcm_choqok_is_gd SOURCES is_gd_config.cpp INSTALL_NAMESPACE "choqok_plugins/kcms")

kconfig_add_kcfg_files(kcm_choqok_is_gd is_gd_settings.kcfgc)

ki18n_wrap_ui(kcm_choqok_is_gd is_gd_prefs.ui)

target_link_libraries(kcm_choqok_is_gd
PUBLIC
    Qt::Core
    KF5::ConfigWidgets
    KF5::CoreAddons
    KF5::I18n
    choqok
)
