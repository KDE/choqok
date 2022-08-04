add_library(choqok_untiny MODULE
    untiny.cpp
)

kconfig_add_kcfg_files(choqok_untiny untinysettings.kcfgc)

kcoreaddons_desktop_to_json(choqok_untiny choqok_untiny.desktop)

target_link_libraries(choqok_untiny
PUBLIC
    Qt::Core
    KF5::CoreAddons
    KF5::KIOCore
    choqok
)

install(TARGETS choqok_untiny DESTINATION ${KDE_INSTALL_PLUGINDIR})


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
# install(TARGETS kcm_choqok_untiny DESTINATION ${KDE_INSTALL_PLUGINDIR})

########### next target ###############


install( FILES untinysettings.kcfg  DESTINATION ${KDE_INSTALL_KCFGDIR})
install( FILES choqok_untiny.desktop #choqok_untiny_config.desktop
         DESTINATION ${KDE_INSTALL_KSERVICESDIR})
