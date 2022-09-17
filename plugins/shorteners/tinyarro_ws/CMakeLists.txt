kcoreaddons_add_plugin(choqok_tinyarro_ws SOURCES tinyarro_ws.cpp INSTALL_NAMESPACE "choqok_plugins")

kconfig_add_kcfg_files(choqok_tinyarro_ws tinyarro_ws_settings.kcfgc)

target_link_libraries(choqok_tinyarro_ws
PUBLIC
    Qt::Core
    KF5::CoreAddons
    KF5::I18n
    KF5::KIOCore
    choqok
)

kcoreaddons_add_plugin(kcm_choqok_tinyarro_ws SOURCES tinyarro_ws_config.cpp INSTALL_NAMESPACE "choqok_plugins/kcms")

ki18n_wrap_ui(kcm_choqok_tinyarro_ws tinyarro_ws_prefs.ui)

kconfig_add_kcfg_files(kcm_choqok_tinyarro_ws tinyarro_ws_settings.kcfgc)

target_link_libraries(kcm_choqok_tinyarro_ws
PUBLIC
    Qt::Core
    KF5::ConfigCore
    KF5::ConfigWidgets
    KF5::CoreAddons
    KF5::I18n
    choqok
)
