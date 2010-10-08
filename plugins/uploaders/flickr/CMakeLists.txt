
include_directories(
${CHOQOK_INCLUDES}
)

add_subdirectory(icons)

########### next target ###############

set( choqok_flickr_PART_SRCS
flickr.cpp)

kde4_add_kcfg_files(choqok_flickr_PART_SRCS flickrsettings.kcfgc )

kde4_add_plugin(choqok_flickr ${choqok_flickr_PART_SRCS})

target_link_libraries(choqok_flickr ${KDE4_KIO_LIBS} choqok)

install(TARGETS choqok_flickr DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

set( kcm_choqok_flickr_PART_SRCS
flickrconfig.cpp )

kde4_add_ui_files(kcm_choqok_flickr_PART_SRCS flickrprefs.ui )

kde4_add_kcfg_files(kcm_choqok_flickr_PART_SRCS flickrsettings.kcfgc )

kde4_add_plugin(kcm_choqok_flickr ${kcm_choqok_flickr_PART_SRCS})

target_link_libraries(kcm_choqok_flickr ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} choqok)

install(TARGETS kcm_choqok_flickr DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

install( FILES flickrsettings.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES choqok_flickr_config.desktop choqok_flickr.desktop
         DESTINATION ${SERVICES_INSTALL_DIR})
