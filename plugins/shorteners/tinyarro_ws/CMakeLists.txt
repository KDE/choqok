add_library(choqok_tinyarro_ws MODULE
    tinyarro_ws.cpp
)

kconfig_add_kcfg_files(choqok_tinyarro_ws tinyarro_ws_settings.kcfgc)

kcoreaddons_desktop_to_json(choqok_tinyarro_ws choqok_tinyarro_ws.desktop)

target_link_libraries(choqok_tinyarro_ws
PUBLIC
    Qt::Core
    KF5::CoreAddons
    KF5::I18n
    KF5::KIOCore
    choqok
)

install(TARGETS choqok_tinyarro_ws DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES choqok_tinyarro_ws.desktop DESTINATION ${KDE_INSTALL_KSERVICESDIR})

add_library(kcm_choqok_tinyarro_ws MODULE
    tinyarro_ws_config.cpp
)

ki18n_wrap_ui(kcm_choqok_tinyarro_ws tinyarro_ws_prefs.ui)

kconfig_add_kcfg_files(kcm_choqok_tinyarro_ws tinyarro_ws_settings.kcfgc)

kcoreaddons_desktop_to_json(kcm_choqok_tinyarro_ws choqok_tinyarro_ws_config.desktop)

target_link_libraries(kcm_choqok_tinyarro_ws
PUBLIC
    Qt::Core
    KF5::ConfigCore
    KF5::ConfigWidgets
    KF5::CoreAddons
    KF5::I18n
    choqok
)

install(TARGETS kcm_choqok_tinyarro_ws DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES tinyarro_ws_settings.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
install(FILES choqok_tinyarro_ws_config.desktop DESTINATION ${KDE_INSTALL_KSERVICESDIR})
