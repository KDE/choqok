
include_directories(
${CHOQOK_INCLUDES}
)

########### next target ###############

set( choqok_twitpic_PART_SRCS
twitpic.cpp)

# kde4_add_ui_files(choqok_twitpic_PART_SRCS twitpicuploadimage_base.ui)

kde4_add_kcfg_files(choqok_twitpic_PART_SRCS twitpicsettings.kcfgc )

kde4_add_plugin(choqok_twitpic ${choqok_twitpic_PART_SRCS})

target_link_libraries(choqok_twitpic ${KDE4_KIO_LIBS} choqok)

install(TARGETS choqok_twitpic DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

set( kcm_choqok_twitpic_PART_SRCS
twitpicconfig.cpp )

kde4_add_ui_files(kcm_choqok_twitpic_PART_SRCS twitpicprefs.ui )

kde4_add_kcfg_files(kcm_choqok_twitpic_PART_SRCS twitpicsettings.kcfgc )

kde4_add_plugin(kcm_choqok_twitpic ${kcm_choqok_twitpic_PART_SRCS})

target_link_libraries(kcm_choqok_twitpic ${KDE4_KDEUI_LIBS} choqok)

install(TARGETS kcm_choqok_twitpic DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

install( FILES twitpicsettings.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES choqok_twitpic_config.desktop choqok_twitpic.desktop
         DESTINATION ${SERVICES_INSTALL_DIR})
# install( FILES twitpicui.rc  DESTINATION  ${DATA_INSTALL_DIR}/choqok_twitpic )
