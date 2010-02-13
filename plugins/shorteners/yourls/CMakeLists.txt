include_directories(
${CHOQOK_INCLUDES}
)

########### next target ###############

set( choqok_yourls_SRCS
yourls.cpp )

kde4_add_kcfg_files(choqok_yourls_SRCS yourlssettings.kcfgc )

kde4_add_plugin(choqok_yourls ${choqok_yourls_SRCS})

target_link_libraries(choqok_yourls ${KDE4_KIO_LIBS} choqok)

install(TARGETS choqok_yourls DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES choqok_yourls.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

########### next target ###############

set( kcm_choqok_yourls_PART_SRCS
yourlsconfig.cpp )

kde4_add_ui_files(kcm_choqok_yourls_PART_SRCS yourlsprefs.ui )

kde4_add_kcfg_files(kcm_choqok_yourls_PART_SRCS yourlssettings.kcfgc )

kde4_add_plugin(kcm_choqok_yourls ${kcm_choqok_yourls_PART_SRCS})

target_link_libraries(kcm_choqok_yourls ${KDE4_KDEUI_LIBS} choqok)

install(TARGETS kcm_choqok_yourls DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES yourlssettings.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES choqok_yourls_config.desktop
         DESTINATION ${SERVICES_INSTALL_DIR})