add_library(choqok_twitgoo MODULE
    twitgoo.cpp
)

kconfig_add_kcfg_files(choqok_twitgoo twitgoosettings.kcfgc)

kcoreaddons_desktop_to_json(choqok_twitgoo choqok_twitgoo.desktop)

target_link_libraries(choqok_twitgoo
PUBLIC
    Qt::Core
    Qt::NetworkAuth
    KF5::CoreAddons
    KF5::KIOCore
    choqok
    twitterapihelper
)

install(TARGETS choqok_twitgoo DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES choqok_twitgoo.desktop DESTINATION ${KDE_INSTALL_KSERVICESDIR})

add_library(kcm_choqok_twitgoo MODULE
    twitgooconfig.cpp
)

ki18n_wrap_ui(kcm_choqok_twitgoo twitgooprefs.ui)

kconfig_add_kcfg_files(kcm_choqok_twitgoo twitgoosettings.kcfgc)

kcoreaddons_desktop_to_json(kcm_choqok_twitgoo choqok_twitgoo_config.desktop)

target_link_libraries(kcm_choqok_twitgoo
PUBLIC
    Qt::Widgets
    KF5::CoreAddons
    KF5::I18n
    KF5::WidgetsAddons
    choqok
)

install(TARGETS kcm_choqok_twitgoo DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES twitgoosettings.kcfg  DESTINATION ${KDE_INSTALL_KCFGDIR})
install(FILES choqok_twitgoo_config.desktop DESTINATION ${KDE_INSTALL_KSERVICESDIR})

add_subdirectory(icons)
