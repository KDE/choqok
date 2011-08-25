include_directories(
${CHOQOK_INCLUDES}
)

set( choqok_is_gd_SRCS
is_gd.cpp )

kde4_add_kcfg_files(choqok_is_gd_SRCS is_gd_settings.kcfgc )

kde4_add_plugin(choqok_is_gd ${choqok_is_gd_SRCS})

target_link_libraries(choqok_is_gd ${KDE4_KIO_LIBS} ${QJSON_LIBRARY} choqok)

install(TARGETS choqok_is_gd DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES choqok_is_gd.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
 
########### next target ###############

set( kcm_choqok_is_gd_PART_SRCS
	is_gd_config.cpp )

kde4_add_ui_files(kcm_choqok_is_gd_PART_SRCS is_gd_prefs.ui )

kde4_add_kcfg_files(kcm_choqok_is_gd_PART_SRCS is_gd_settings.kcfgc )

kde4_add_plugin(kcm_choqok_is_gd ${kcm_choqok_is_gd_PART_SRCS})

target_link_libraries(kcm_choqok_is_gd ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS}
	choqok)

install( TARGETS kcm_choqok_is_gd DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES is_gd_settings.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES choqok_is_gd_config.desktop
	         DESTINATION ${SERVICES_INSTALL_DIR})

