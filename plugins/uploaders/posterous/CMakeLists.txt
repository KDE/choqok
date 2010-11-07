
include_directories(
${CHOQOK_INCLUDES}
)

########### next target ###############

set( choqok_posterous_PART_SRCS
posterous.cpp)

kde4_add_kcfg_files(choqok_posterous_PART_SRCS posteroussettings.kcfgc )

kde4_add_plugin(choqok_posterous ${choqok_posterous_PART_SRCS})

target_link_libraries(choqok_posterous ${KDE4_KIO_LIBS} ${QTOAUTH_LIBRARY} ${QJSON_LIBRARY} choqok twitterapihelper)

install(TARGETS choqok_posterous DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

set( kcm_choqok_posterous_PART_SRCS
posterousconfig.cpp )

kde4_add_ui_files(kcm_choqok_posterous_PART_SRCS posterousprefs.ui )

kde4_add_kcfg_files(kcm_choqok_posterous_PART_SRCS posteroussettings.kcfgc )

kde4_add_plugin(kcm_choqok_posterous ${kcm_choqok_posterous_PART_SRCS})

target_link_libraries(kcm_choqok_posterous ${KDE4_KDEUI_LIBS} choqok)

install(TARGETS kcm_choqok_posterous DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

install( FILES posteroussettings.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES choqok_posterous_config.desktop choqok_posterous.desktop
         DESTINATION ${SERVICES_INSTALL_DIR})
