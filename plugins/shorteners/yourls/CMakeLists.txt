set( choqok_yourls_SRCS
yourls.cpp )

kconfig_add_kcfg_files(choqok_yourls_SRCS yourlssettings.kcfgc )

add_library(choqok_yourls MODULE ${choqok_yourls_SRCS})

kcoreaddons_desktop_to_json(choqok_yourls choqok_yourls.desktop)

target_link_libraries(choqok_yourls
PUBLIC
    Qt5::Core
    KF5::CoreAddons
    KF5::I18n
    KF5::KIOCore
    KF5::KDELibs4Support
    choqok
)

install(TARGETS choqok_yourls DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES choqok_yourls.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

########### next target ###############

set( kcm_choqok_yourls_PART_SRCS
yourlsconfig.cpp )

ki18n_wrap_ui(kcm_choqok_yourls_PART_SRCS yourlsprefs.ui )

kconfig_add_kcfg_files(kcm_choqok_yourls_PART_SRCS yourlssettings.kcfgc )

add_library(kcm_choqok_yourls MODULE ${kcm_choqok_yourls_PART_SRCS})

kcoreaddons_desktop_to_json(kcm_choqok_yourls choqok_yourls_config.desktop)

target_link_libraries(kcm_choqok_yourls
PUBLIC
    Qt5::Core
    KF5::ConfigWidgets
    KF5::CoreAddons
    KF5::I18n
    choqok
)

install(TARGETS kcm_choqok_yourls DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES yourlssettings.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES choqok_yourls_config.desktop
         DESTINATION ${SERVICES_INSTALL_DIR})