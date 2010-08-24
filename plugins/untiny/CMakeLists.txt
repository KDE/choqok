
include_directories(
${CHOQOK_INCLUDES}
)

########### next target ###############

set( choqok_untiny_PART_SRCS
untiny.cpp )

kde4_add_kcfg_files(choqok_untiny_PART_SRCS untinysettings.kcfgc)

kde4_add_plugin(choqok_untiny ${choqok_untiny_PART_SRCS})

target_link_libraries(choqok_untiny ${KDE4_KIO_LIBS} choqok)

install(TARGETS choqok_untiny DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set( kcm_choqok_untiny_PART_SRCS
untinyconfig.cpp )

kde4_add_ui_files(kcm_choqok_untiny_PART_SRCS untinyprefs.ui )

kde4_add_kcfg_files(kcm_choqok_untiny_PART_SRCS untinysettings.kcfgc )

kde4_add_plugin(kcm_choqok_untiny ${kcm_choqok_untiny_PART_SRCS})

target_link_libraries(kcm_choqok_untiny ${KDE4_KDEUI_LIBS})

install(TARGETS kcm_choqok_untiny DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############


install( FILES untinysettings.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES choqok_untiny.desktop choqok_untiny_config.desktop
         DESTINATION ${SERVICES_INSTALL_DIR})
