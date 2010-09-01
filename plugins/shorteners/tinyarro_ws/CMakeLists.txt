include_directories(
${CHOQOK_INCLUDES}
${CMAKE_CURRENT_BINARY_DIR}
)

########### next target ###############

set( choqok_tinyarro_ws_SRCS
tinyarro_ws.cpp )

kde4_add_kcfg_files(choqok_tinyarro_ws_SRCS tinyarro_ws_settings.kcfgc )

kde4_add_plugin(choqok_tinyarro_ws ${choqok_tinyarro_ws_SRCS})

target_link_libraries(choqok_tinyarro_ws ${KDE4_KIO_LIBS} choqok)

install(TARGETS choqok_tinyarro_ws DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES choqok_tinyarro_ws.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

########### next target ###############

set( kcm_choqok_tinyarro_ws_PART_SRCS
tinyarro_ws_config.cpp )

kde4_add_ui_files(kcm_choqok_tinyarro_ws_PART_SRCS tinyarro_ws_prefs.ui )

kde4_add_kcfg_files(kcm_choqok_tinyarro_ws_PART_SRCS tinyarro_ws_settings.kcfgc )

kde4_add_plugin(kcm_choqok_tinyarro_ws ${kcm_choqok_tinyarro_ws_PART_SRCS})

target_link_libraries(kcm_choqok_tinyarro_ws ${KDE4_KDEUI_LIBS} choqok)

install(TARGETS kcm_choqok_tinyarro_ws DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES tinyarro_ws_settings.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES choqok_tinyarro_ws_config.desktop
         DESTINATION ${SERVICES_INSTALL_DIR})