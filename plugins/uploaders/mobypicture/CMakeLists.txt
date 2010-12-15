
include_directories(
${CHOQOK_INCLUDES}
)

########### next target ###############

set( choqok_mobypicture_PART_SRCS
mobypicture.cpp)

kde4_add_kcfg_files(choqok_mobypicture_PART_SRCS mobypicturesettings.kcfgc )

kde4_add_plugin(choqok_mobypicture ${choqok_mobypicture_PART_SRCS})

target_link_libraries(choqok_mobypicture ${KDE4_KIO_LIBS} ${QTOAUTH_LIBRARY} ${QJSON_LIBRARY} choqok twitterapihelper)

install(TARGETS choqok_mobypicture DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

set( kcm_choqok_mobypicture_PART_SRCS
mobypictureconfig.cpp )

kde4_add_ui_files(kcm_choqok_mobypicture_PART_SRCS mobypictureprefs.ui )

kde4_add_kcfg_files(kcm_choqok_mobypicture_PART_SRCS mobypicturesettings.kcfgc )

kde4_add_plugin(kcm_choqok_mobypicture ${kcm_choqok_mobypicture_PART_SRCS})

target_link_libraries(kcm_choqok_mobypicture ${KDE4_KDEUI_LIBS} choqok)

install(TARGETS kcm_choqok_mobypicture DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

install( FILES mobypicturesettings.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES choqok_mobypicture_config.desktop choqok_mobypicture.desktop
         DESTINATION ${SERVICES_INSTALL_DIR})

add_subdirectory(icons)
