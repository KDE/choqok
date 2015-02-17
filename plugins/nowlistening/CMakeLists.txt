
include_directories(
${CHOQOK_INCLUDES}
)

########### next target ###############

set( choqok_nowlistening_PART_SRCS mpris.cpp
nowlistening.cpp )

kconfig_add_kcfg_files(choqok_nowlistening_PART_SRCS nowlisteningsettings.kcfgc )

add_library(choqok_nowlistening MODULE ${choqok_nowlistening_PART_SRCS})

target_link_libraries(choqok_nowlistening KF5::KIOCore choqok)

install(TARGETS choqok_nowlistening DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

set( kcm_choqok_nowlistening_PART_SRCS
nowlisteningconfig.cpp )

ki18n_wrap_ui(kcm_choqok_nowlistening_PART_SRCS nowlisteningprefs.ui )

kconfig_add_kcfg_files(kcm_choqok_nowlistening_PART_SRCS nowlisteningsettings.kcfgc )

add_library(kcm_choqok_nowlistening MODULE ${kcm_choqok_nowlistening_PART_SRCS})

target_link_libraries(kcm_choqok_nowlistening )

install(TARGETS kcm_choqok_nowlistening DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

install( FILES nowlisteningsettings.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES choqok_nowlistening_config.desktop choqok_nowlistening.desktop
         DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES nowlisteningui.rc  DESTINATION  ${DATA_INSTALL_DIR}/choqok_nowlistening )
