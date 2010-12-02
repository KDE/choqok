
include_directories(
${CHOQOK_INCLUDES}
)

########### next target ###############

set( choqok_imstatus_PART_SRCS imqdbus.cpp
imstatus.cpp )

kde4_add_kcfg_files(choqok_imstatus_PART_SRCS imstatussettings.kcfgc )

kde4_add_plugin(choqok_imstatus ${choqok_imstatus_PART_SRCS})

target_link_libraries(choqok_imstatus ${KDE4_KIO_LIBS} choqok)

install(TARGETS choqok_imstatus DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

set( kcm_choqok_imstatus_PART_SRCS imqdbus.cpp
imstatusconfig.cpp )

kde4_add_ui_files(kcm_choqok_imstatus_PART_SRCS imstatusprefs.ui )

kde4_add_kcfg_files(kcm_choqok_imstatus_PART_SRCS imstatussettings.kcfgc )

kde4_add_plugin(kcm_choqok_imstatus ${kcm_choqok_imstatus_PART_SRCS})

target_link_libraries(kcm_choqok_imstatus ${KDE4_KDEUI_LIBS})

install(TARGETS kcm_choqok_imstatus DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

install( FILES imstatussettings.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES choqok_imstatus_config.desktop choqok_imstatus.desktop
         DESTINATION ${SERVICES_INSTALL_DIR})
