
include_directories(
${CHOQOK_INCLUDES}
)

########### next target ###############

set( choqok_twitgoo_PART_SRCS
twitgoo.cpp)

kde4_add_kcfg_files(choqok_twitgoo_PART_SRCS twitgoosettings.kcfgc )

kde4_add_plugin(choqok_twitgoo ${choqok_twitgoo_PART_SRCS})

target_link_libraries(choqok_twitgoo ${KDE4_KIO_LIBS} ${QTOAUTH_LIBRARY} ${QJSON_LIBRARY} choqok twitterapihelper)

install(TARGETS choqok_twitgoo DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

set( kcm_choqok_twitgoo_PART_SRCS
twitgooconfig.cpp )

kde4_add_ui_files(kcm_choqok_twitgoo_PART_SRCS twitgooprefs.ui )

kde4_add_kcfg_files(kcm_choqok_twitgoo_PART_SRCS twitgoosettings.kcfgc )

kde4_add_plugin(kcm_choqok_twitgoo ${kcm_choqok_twitgoo_PART_SRCS})

target_link_libraries(kcm_choqok_twitgoo ${KDE4_KDEUI_LIBS} choqok)

install(TARGETS kcm_choqok_twitgoo DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

install( FILES twitgoosettings.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES choqok_twitgoo_config.desktop choqok_twitgoo.desktop
         DESTINATION ${SERVICES_INSTALL_DIR})

