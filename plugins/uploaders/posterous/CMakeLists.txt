
include_directories(
${CHOQOK_INCLUDES}
)

########### next target ###############

set( choqok_posterous_PART_SRCS
posterous.cpp)

kconfig_add_kcfg_files(choqok_posterous_PART_SRCS posteroussettings.kcfgc )

add_library(choqok_posterous MODULE ${choqok_posterous_PART_SRCS})

target_link_libraries(choqok_posterous KF5::KIOCore ${QTOAUTH_LIBRARY} ${QJSON_LIBRARY} choqok twitterapihelper)

install(TARGETS choqok_posterous DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

set( kcm_choqok_posterous_PART_SRCS
posterousconfig.cpp )

ki18n_wrap_ui(kcm_choqok_posterous_PART_SRCS posterousprefs.ui )

kconfig_add_kcfg_files(kcm_choqok_posterous_PART_SRCS posteroussettings.kcfgc )

add_library(kcm_choqok_posterous MODULE ${kcm_choqok_posterous_PART_SRCS})

target_link_libraries(kcm_choqok_posterous  choqok)

install(TARGETS kcm_choqok_posterous DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

install( FILES posteroussettings.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES choqok_posterous_config.desktop choqok_posterous.desktop
         DESTINATION ${SERVICES_INSTALL_DIR})

add_subdirectory(icons)
