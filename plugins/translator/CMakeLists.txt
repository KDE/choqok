add_library(choqok_translator MODULE
    sharedtools.cpp
    translator.cpp
)

kconfig_add_kcfg_files(choqok_translator translatorsettings.kcfgc)

target_link_libraries(choqok_translator  KF5::KIOCore ${QJSON_LIBRARY} choqok)

install(TARGETS choqok_translator DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES languagecodes DESTINATION ${DATA_INSTALL_DIR}/choqok)

########### next target ###############

add_library(kcm_choqok_translator MODULE
    translatorconfig.cpp
    sharedtools.cpp
)

ki18n_wrap_ui(kcm_choqok_translator translatorprefs.ui )

kconfig_add_kcfg_files(kcm_choqok_translator translatorsettings.kcfgc )

target_link_libraries(kcm_choqok_translator )

install(TARGETS kcm_choqok_translator DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############


install( FILES translatorsettings.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES choqok_translator.desktop choqok_translator_config.desktop
         DESTINATION ${SERVICES_INSTALL_DIR})
