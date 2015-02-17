
include_directories(
${CHOQOK_INCLUDES}
)

add_subdirectory(icons)

########### next target ###############

set( choqok_flickr_PART_SRCS
flickr.cpp)

kconfig_add_kcfg_files(choqok_flickr_PART_SRCS flickrsettings.kcfgc )

add_library(choqok_flickr MODULE ${choqok_flickr_PART_SRCS})

target_link_libraries(choqok_flickr KF5::KIOCore choqok)

install(TARGETS choqok_flickr DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

set( kcm_choqok_flickr_PART_SRCS
flickrconfig.cpp )

ki18n_wrap_ui(kcm_choqok_flickr_PART_SRCS flickrprefs.ui )

kconfig_add_kcfg_files(kcm_choqok_flickr_PART_SRCS flickrsettings.kcfgc )

add_library(kcm_choqok_flickr MODULE ${kcm_choqok_flickr_PART_SRCS})

target_link_libraries(kcm_choqok_flickr  KF5::KIOCore choqok)

install(TARGETS kcm_choqok_flickr DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

install( FILES flickrsettings.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES choqok_flickr_config.desktop choqok_flickr.desktop
         DESTINATION ${SERVICES_INSTALL_DIR})
