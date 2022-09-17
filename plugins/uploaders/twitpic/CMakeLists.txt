kcoreaddons_add_plugin(choqok_twitpic SOURCES twitpic.cpp INSTALL_NAMESPACE "choqok_plugins")

kconfig_add_kcfg_files(choqok_twitpic twitpicsettings.kcfgc)
# ki18n_wrap_ui(choqok_twitpic twitpicuploadimage_base.ui)

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

kcoreaddons_add_plugin(kcm_choqok_twitpic SOURCES twitpicconfig.cpp INSTALL_NAMESPACE "choqok_plugins/kcms")

ki18n_wrap_ui(kcm_choqok_twitpic twitpicprefs.ui)
kconfig_add_kcfg_files(kcm_choqok_twitpic twitpicsettings.kcfgc )

target_link_libraries(kcm_choqok_twitpic choqok)

install(FILES twitpicsettings.kcfg  DESTINATION ${KDE_INSTALL_KCFGDIR})
# install( FILES twitpicui.rc  DESTINATION  ${KDE_INSTALL_DATADIR}/choqok_twitpic )

add_subdirectory(icons)
