set( choqok_bit_ly_SRCS
bit_ly.cpp )

kconfig_add_kcfg_files(choqok_bit_ly_SRCS bit_ly_settings.kcfgc )

add_library(choqok_bit_ly MODULE ${choqok_bit_ly_SRCS})

kcoreaddons_desktop_to_json(choqok_bit_ly choqok_bit_ly.desktop)

target_link_libraries(choqok_bit_ly
PUBLIC
    Qt5::Core
    KF5::CoreAddons
    KF5::I18n
    KF5::KIOCore
    KF5::KDELibs4Support
    choqok
)

install( TARGETS choqok_bit_ly DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES choqok_bit_ly.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

########### next target ###############

set( kcm_choqok_bit_ly_PART_SRCS
bit_ly_config.cpp )

ki18n_wrap_ui(kcm_choqok_bit_ly_PART_SRCS bit_ly_prefs.ui )

kconfig_add_kcfg_files(kcm_choqok_bit_ly_PART_SRCS bit_ly_settings.kcfgc )

add_library(kcm_choqok_bit_ly MODULE ${kcm_choqok_bit_ly_PART_SRCS})

kcoreaddons_desktop_to_json(kcm_choqok_bit_ly choqok_bit_ly_config.desktop)

target_link_libraries(kcm_choqok_bit_ly
PUBLIC
    Qt5::Core
    KF5::ConfigCore
    KF5::ConfigWidgets
    KF5::CoreAddons
    KF5::I18n
    KF5::KIOCore
    KF5::WidgetsAddons
    KF5::KDELibs4Support
    choqok
)

install( TARGETS kcm_choqok_bit_ly DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES bit_ly_settings.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES choqok_bit_ly_config.desktop
         DESTINATION ${SERVICES_INSTALL_DIR})
