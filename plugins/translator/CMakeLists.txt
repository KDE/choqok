add_library(choqok_translator MODULE
    sharedtools.cpp
    translator.cpp
)

kconfig_add_kcfg_files(choqok_translator translatorsettings.kcfgc)

target_link_libraries(choqok_translator  KF5::KIOCore ${QJSON_LIBRARY} choqok)

install(TARGETS choqok_translator DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES languagecodes DESTINATION ${KDE_INSTALL_DATADIR}/choqok)

########### next target ###############

add_library(kcm_choqok_translator MODULE
    translatorconfig.cpp
    sharedtools.cpp
)

ki18n_wrap_ui(kcm_choqok_translator translatorprefs.ui )

kconfig_add_kcfg_files(kcm_choqok_translator translatorsettings.kcfgc )

target_link_libraries(kcm_choqok_translator )

install(TARGETS kcm_choqok_translator DESTINATION ${KDE_INSTALL_PLUGINDIR})

########### next target ###############


install( FILES translatorsettings.kcfg  DESTINATION ${KDE_INSTALL_KCFGDIR})
install( FILES choqok_translator.desktop choqok_translator_config.desktop
         DESTINATION ${KDE_INSTALL_KSERVICESDIR})
