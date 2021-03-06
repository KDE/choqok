include_directories(
    ${CHOQOK_INCLUDES}
)

set(choqok_twitgoo_PART_SRCS
    twitgoo.cpp
)

kconfig_add_kcfg_files(choqok_twitgoo_PART_SRCS twitgoosettings.kcfgc)

add_library(choqok_twitgoo MODULE ${choqok_twitgoo_PART_SRCS})

kcoreaddons_desktop_to_json(choqok_twitgoo choqok_twitgoo.desktop)

target_link_libraries(choqok_twitgoo
PUBLIC
    Qt5::Core
    Qt5::NetworkAuth
    KF5::CoreAddons
    KF5::KIOCore
    choqok
    twitterapihelper
)

install(TARGETS choqok_twitgoo DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES choqok_twitgoo.desktop DESTINATION ${SERVICES_INSTALL_DIR})

set(kcm_choqok_twitgoo_PART_SRCS
    twitgooconfig.cpp
)

ki18n_wrap_ui(kcm_choqok_twitgoo_PART_SRCS twitgooprefs.ui)

kconfig_add_kcfg_files(kcm_choqok_twitgoo_PART_SRCS twitgoosettings.kcfgc)

add_library(kcm_choqok_twitgoo MODULE ${kcm_choqok_twitgoo_PART_SRCS})

kcoreaddons_desktop_to_json(kcm_choqok_twitgoo choqok_twitgoo_config.desktop)

target_link_libraries(kcm_choqok_twitgoo
PUBLIC
    Qt5::Widgets
    KF5::CoreAddons
    KF5::I18n
    KF5::WidgetsAddons
    choqok
)

install(TARGETS kcm_choqok_twitgoo DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES twitgoosettings.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install(FILES choqok_twitgoo_config.desktop DESTINATION ${SERVICES_INSTALL_DIR})

add_subdirectory(icons)
