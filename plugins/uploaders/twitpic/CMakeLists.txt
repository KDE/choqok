add_library(choqok_twitpic MODULE
    twitpic.cpp
)

# ki18n_wrap_ui(choqok_twitpic twitpicuploadimage_base.ui)

kconfig_add_kcfg_files(choqok_twitpic twitpicsettings.kcfgc)

kcoreaddons_desktop_to_json(choqok_twitpic choqok_twitpic.desktop)

target_link_libraries(choqok_twitpic
PUBLIC
    Qt5::Core
    Qt5::NetworkAuth
    KF5::CoreAddons
    KF5::I18n
    KF5::KIOCore
    choqok
    twitterapihelper
)

install(TARGETS choqok_twitpic DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES choqok_twitpic.desktop DESTINATION ${SERVICES_INSTALL_DIR})

add_library(kcm_choqok_twitpic MODULE
    twitpicconfig.cpp
)

ki18n_wrap_ui(kcm_choqok_twitpic twitpicprefs.ui)

kconfig_add_kcfg_files(kcm_choqok_twitpic twitpicsettings.kcfgc )

kcoreaddons_desktop_to_json(kcm_choqok_twitpic choqok_twitpic_config.desktop)

target_link_libraries(kcm_choqok_twitpic choqok)

install(TARGETS kcm_choqok_twitpic DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES twitpicsettings.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install(FILES choqok_twitpic_config.desktop DESTINATION ${SERVICES_INSTALL_DIR})
# install( FILES twitpicui.rc  DESTINATION  ${DATA_INSTALL_DIR}/choqok_twitpic )

add_subdirectory(icons)
