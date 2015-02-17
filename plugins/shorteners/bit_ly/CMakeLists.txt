
include_directories(
${CHOQOK_INCLUDES}
)

########### next target ###############

set( choqok_bit_ly_SRCS
bit_ly.cpp )

kconfig_add_kcfg_files(choqok_bit_ly_SRCS bit_ly_settings.kcfgc )

add_library(choqok_bit_ly MODULE ${choqok_bit_ly_SRCS})

target_link_libraries(choqok_bit_ly KF5::KIOCore choqok)

install( TARGETS choqok_bit_ly DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES choqok_bit_ly.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

########### next target ###############

set( kcm_choqok_bit_ly_PART_SRCS
bit_ly_config.cpp )

ki18n_wrap_ui(kcm_choqok_bit_ly_PART_SRCS bit_ly_prefs.ui )

kconfig_add_kcfg_files(kcm_choqok_bit_ly_PART_SRCS bit_ly_settings.kcfgc )

add_library(kcm_choqok_bit_ly MODULE ${kcm_choqok_bit_ly_PART_SRCS})

target_link_libraries(kcm_choqok_bit_ly  KF5::KIOCore choqok)

install( TARGETS kcm_choqok_bit_ly DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES bit_ly_settings.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES choqok_bit_ly_config.desktop
         DESTINATION ${SERVICES_INSTALL_DIR})
