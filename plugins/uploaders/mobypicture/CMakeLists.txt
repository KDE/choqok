
include_directories(
${CHOQOK_INCLUDES}
)

########### next target ###############

set( choqok_mobypicture_PART_SRCS
mobypicture.cpp)

kconfig_add_kcfg_files(choqok_mobypicture_PART_SRCS mobypicturesettings.kcfgc )

add_library(choqok_mobypicture MODULE ${choqok_mobypicture_PART_SRCS})

target_link_libraries(choqok_mobypicture KF5::KIOCore ${QTOAUTH_LIBRARY} ${QJSON_LIBRARY} choqok twitterapihelper)

install(TARGETS choqok_mobypicture DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

set( kcm_choqok_mobypicture_PART_SRCS
mobypictureconfig.cpp )

ki18n_wrap_ui(kcm_choqok_mobypicture_PART_SRCS mobypictureprefs.ui )

kconfig_add_kcfg_files(kcm_choqok_mobypicture_PART_SRCS mobypicturesettings.kcfgc )

add_library(kcm_choqok_mobypicture MODULE ${kcm_choqok_mobypicture_PART_SRCS})

target_link_libraries(kcm_choqok_mobypicture  choqok)

install(TARGETS kcm_choqok_mobypicture DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

install( FILES mobypicturesettings.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES choqok_mobypicture_config.desktop choqok_mobypicture.desktop
         DESTINATION ${SERVICES_INSTALL_DIR})

add_subdirectory(icons)
