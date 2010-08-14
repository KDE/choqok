
include_directories(
${CHOQOK_INCLUDES}
)

add_subdirectory(icons)
########### next target ###############

set( choqok_yfrog_PART_SRCS
yfrog.cpp)

kde4_add_kcfg_files(choqok_yfrog_PART_SRCS yfrogsettings.kcfgc )

kde4_add_plugin(choqok_yfrog ${choqok_yfrog_PART_SRCS})

target_link_libraries(choqok_yfrog ${KDE4_KIO_LIBS} choqok)

install(TARGETS choqok_yfrog DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

set( kcm_choqok_yfrog_PART_SRCS
yfrogconfig.cpp )

kde4_add_ui_files(kcm_choqok_yfrog_PART_SRCS yfrogprefs.ui )

kde4_add_kcfg_files(kcm_choqok_yfrog_PART_SRCS yfrogsettings.kcfgc )

kde4_add_plugin(kcm_choqok_yfrog ${kcm_choqok_yfrog_PART_SRCS})

target_link_libraries(kcm_choqok_yfrog ${KDE4_KDEUI_LIBS} choqok)

install(TARGETS kcm_choqok_yfrog DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

install( FILES yfrogsettings.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES choqok_yfrog_config.desktop choqok_yfrog.desktop
         DESTINATION ${SERVICES_INSTALL_DIR})
