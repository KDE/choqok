
include_directories(
${CHOQOK_INCLUDES}
)

########### next target ###############

set( choqok_translator_PART_SRCS
sharedtools.cpp
translator.cpp )

kde4_add_kcfg_files(choqok_translator_PART_SRCS translatorsettings.kcfgc)

kde4_add_plugin(choqok_translator ${choqok_translator_PART_SRCS})

target_link_libraries(choqok_translator ${KDE4_KUTILS_LIBS} ${KDE4_KIO_LIBS} ${QJSON_LIBRARY} choqok)

install(TARGETS choqok_translator DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES languagecodes DESTINATION ${DATA_INSTALL_DIR}/choqok)

########### next target ###############

set( kcm_choqok_translator_PART_SRCS
translatorconfig.cpp
sharedtools.cpp )

kde4_add_ui_files(kcm_choqok_translator_PART_SRCS translatorprefs.ui )

kde4_add_kcfg_files(kcm_choqok_translator_PART_SRCS translatorsettings.kcfgc )

kde4_add_plugin(kcm_choqok_translator ${kcm_choqok_translator_PART_SRCS})

target_link_libraries(kcm_choqok_translator ${KDE4_KDEUI_LIBS})

install(TARGETS kcm_choqok_translator DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############


install( FILES translatorsettings.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES choqok_translator.desktop choqok_translator_config.desktop
         DESTINATION ${SERVICES_INSTALL_DIR})
