set(choqok_tinyarro_ws_SRCS
    tinyarro_ws.cpp
)

kconfig_add_kcfg_files(choqok_tinyarro_ws_SRCS tinyarro_ws_settings.kcfgc)

add_library(choqok_tinyarro_ws MODULE ${choqok_tinyarro_ws_SRCS})

kcoreaddons_desktop_to_json(choqok_tinyarro_ws choqok_tinyarro_ws.desktop)

target_link_libraries(choqok_tinyarro_ws
PUBLIC
    Qt5::Core
    KF5::CoreAddons
    KF5::I18n
    KF5::KIOCore
    choqok
)

install(TARGETS choqok_tinyarro_ws DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES choqok_tinyarro_ws.desktop DESTINATION ${SERVICES_INSTALL_DIR})

set(kcm_choqok_tinyarro_ws_PART_SRCS
    tinyarro_ws_config.cpp
)

ki18n_wrap_ui(kcm_choqok_tinyarro_ws_PART_SRCS tinyarro_ws_prefs.ui)

kconfig_add_kcfg_files(kcm_choqok_tinyarro_ws_PART_SRCS tinyarro_ws_settings.kcfgc)

add_library(kcm_choqok_tinyarro_ws MODULE ${kcm_choqok_tinyarro_ws_PART_SRCS})

kcoreaddons_desktop_to_json(kcm_choqok_tinyarro_ws choqok_tinyarro_ws_config.desktop)

target_link_libraries(kcm_choqok_tinyarro_ws
PUBLIC
    Qt5::Core
    KF5::ConfigCore
    KF5::ConfigWidgets
    KF5::CoreAddons
    KF5::I18n
    choqok
)

install(TARGETS kcm_choqok_tinyarro_ws DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES tinyarro_ws_settings.kcfg DESTINATION ${KCFG_INSTALL_DIR})
install(FILES choqok_tinyarro_ws_config.desktop DESTINATION ${SERVICES_INSTALL_DIR})
