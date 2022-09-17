kcoreaddons_add_plugin(choqok_twitgoo SOURCES twitgoo.cpp INSTALL_NAMESPACE "choqok_plugins")

kconfig_add_kcfg_files(choqok_twitgoo twitgoosettings.kcfgc)

target_link_libraries(choqok_twitgoo
PUBLIC
    Qt::Core
    Qt::NetworkAuth
    KF5::CoreAddons
    KF5::KIOCore
    choqok
    twitterapihelper
)

kcoreaddons_add_plugin(kcm_choqok_twitgoo SOURCES twitgooconfig.cpp INSTALL_NAMESPACE "choqok_plugins/kcms")

ki18n_wrap_ui(kcm_choqok_twitgoo twitgooprefs.ui)

kconfig_add_kcfg_files(kcm_choqok_twitgoo twitgoosettings.kcfgc)

target_link_libraries(kcm_choqok_twitgoo
PUBLIC
    Qt::Widgets
    KF5::CoreAddons
    KF5::I18n
    KF5::WidgetsAddons
    choqok
)

add_subdirectory(icons)
