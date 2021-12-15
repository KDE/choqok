set(choqok_untiny_PART_SRCS
    untiny.cpp
)

kconfig_add_kcfg_files(choqok_untiny_PART_SRCS untinysettings.kcfgc)

add_library(choqok_untiny MODULE ${choqok_untiny_PART_SRCS})

kcoreaddons_desktop_to_json(choqok_untiny choqok_untiny.desktop)

target_link_libraries(choqok_untiny
PUBLIC
    Qt5::Core
    KF5::CoreAddons
    KF5::KIOCore
    choqok
)

install(TARGETS choqok_untiny DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

# set( kcm_choqok_untiny_PART_SRCS
# untinyconfig.cpp )
#
# ki18n_wrap_ui(kcm_choqok_untiny_PART_SRCS untinyprefs.ui )
#
# kconfig_add_kcfg_files(kcm_choqok_untiny_PART_SRCS untinysettings.kcfgc )
#
# kde4_add_plugin(kcm_choqok_untiny ${kcm_choqok_untiny_PART_SRCS})
#
# target_link_libraries(kcm_choqok_untiny )
#
# install(TARGETS kcm_choqok_untiny DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############


install( FILES untinysettings.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES choqok_untiny.desktop #choqok_untiny_config.desktop
         DESTINATION ${SERVICES_INSTALL_DIR})
