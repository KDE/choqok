
include_directories(
${CHOQOK_INCLUDES}
)

########### next target ###############

set( choqok_bit_ly_SRCS
bit_ly.cpp )

kde4_add_kcfg_files(choqok_bit_ly_SRCS bit_ly_settings.kcfgc )

kde4_add_plugin(choqok_bit_ly ${choqok_bit_ly_SRCS})

target_link_libraries(choqok_bit_ly ${KDE4_KIO_LIBS} choqok)

install( TARGETS choqok_bit_ly DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES choqok_bit_ly.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

########### next target ###############

set( kcm_choqok_bit_ly_PART_SRCS
bit_ly_config.cpp )

kde4_add_ui_files(kcm_choqok_bit_ly_PART_SRCS bit_ly_prefs.ui )

kde4_add_kcfg_files(kcm_choqok_bit_ly_PART_SRCS bit_ly_settings.kcfgc )

kde4_add_plugin(kcm_choqok_bit_ly ${kcm_choqok_bit_ly_PART_SRCS})

target_link_libraries(kcm_choqok_bit_ly ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} choqok)

install( TARGETS kcm_choqok_bit_ly DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES bit_ly_settings.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES choqok_bit_ly_config.desktop
         DESTINATION ${SERVICES_INSTALL_DIR})
