add_library(choqok_twitpic MODULE
    twitpic.cpp
)

# ki18n_wrap_ui(choqok_twitpic twitpicuploadimage_base.ui)

kconfig_add_kcfg_files(choqok_twitpic twitpicsettings.kcfgc)

kcoreaddons_desktop_to_json(choqok_twitpic choqok_twitpic.desktop)

target_link_libraries(choqok_twitpic
PUBLIC
    Qt::Core
    Qt::NetworkAuth
    KF5::CoreAddons
    KF5::I18n
    KF5::KIOCore
    choqok
    twitterapihelper
)

install(TARGETS choqok_twitpic DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES choqok_twitpic.desktop DESTINATION ${KDE_INSTALL_KSERVICESDIR})

add_library(kcm_choqok_twitpic MODULE
    twitpicconfig.cpp
)

ki18n_wrap_ui(kcm_choqok_twitpic twitpicprefs.ui)

kconfig_add_kcfg_files(kcm_choqok_twitpic twitpicsettings.kcfgc )

kcoreaddons_desktop_to_json(kcm_choqok_twitpic choqok_twitpic_config.desktop)

target_link_libraries(kcm_choqok_twitpic choqok)

install(TARGETS kcm_choqok_twitpic DESTINATION ${KDE_INSTALL_PLUGINDIR})

install(FILES twitpicsettings.kcfg  DESTINATION ${KDE_INSTALL_KCFGDIR})
install(FILES choqok_twitpic_config.desktop DESTINATION ${KDE_INSTALL_KSERVICESDIR})
# install( FILES twitpicui.rc  DESTINATION  ${KDE_INSTALL_DATADIR}/choqok_twitpic )

add_subdirectory(icons)
