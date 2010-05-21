
include_directories(
${CHOQOK_INCLUDES}
)

########### next target ###############

set( choqok_nowlistening_PART_SRCS mpris.cpp
nowlistening.cpp )

kde4_add_kcfg_files(choqok_nowlistening_PART_SRCS nowlisteningsettings.kcfgc )

kde4_add_plugin(choqok_nowlistening ${choqok_nowlistening_PART_SRCS})

target_link_libraries(choqok_nowlistening ${KDE4_KIO_LIBS} choqok)

install(TARGETS choqok_nowlistening DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

set( kcm_choqok_nowlistening_PART_SRCS
nowlisteningconfig.cpp )

kde4_add_ui_files(kcm_choqok_nowlistening_PART_SRCS nowlisteningprefs.ui )

kde4_add_kcfg_files(kcm_choqok_nowlistening_PART_SRCS nowlisteningsettings.kcfgc )

kde4_add_plugin(kcm_choqok_nowlistening ${kcm_choqok_nowlistening_PART_SRCS})

target_link_libraries(kcm_choqok_nowlistening ${KDE4_KDEUI_LIBS})

install(TARGETS kcm_choqok_nowlistening DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

install( FILES nowlisteningsettings.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES choqok_nowlistening_config.desktop choqok_nowlistening.desktop
         DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES nowlisteningui.rc  DESTINATION  ${DATA_INSTALL_DIR}/choqok_nowlistening )
