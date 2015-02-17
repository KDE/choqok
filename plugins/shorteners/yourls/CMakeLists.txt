include_directories(
${CHOQOK_INCLUDES}
)

########### next target ###############

set( choqok_yourls_SRCS
yourls.cpp )

kconfig_add_kcfg_files(choqok_yourls_SRCS yourlssettings.kcfgc )

add_library(choqok_yourls MODULE ${choqok_yourls_SRCS})

target_link_libraries(choqok_yourls KF5::KIOCore choqok)

install(TARGETS choqok_yourls DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES choqok_yourls.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

########### next target ###############

set( kcm_choqok_yourls_PART_SRCS
yourlsconfig.cpp )

ki18n_wrap_ui(kcm_choqok_yourls_PART_SRCS yourlsprefs.ui )

kconfig_add_kcfg_files(kcm_choqok_yourls_PART_SRCS yourlssettings.kcfgc )

add_library(kcm_choqok_yourls MODULE ${kcm_choqok_yourls_PART_SRCS})

target_link_libraries(kcm_choqok_yourls  choqok)

install(TARGETS kcm_choqok_yourls DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES yourlssettings.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES choqok_yourls_config.desktop
         DESTINATION ${SERVICES_INSTALL_DIR})