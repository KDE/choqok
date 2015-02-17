
include_directories(
${CHOQOK_INCLUDES}
)

########### next target ###############

set( choqok_twitgoo_PART_SRCS
twitgoo.cpp)

kconfig_add_kcfg_files(choqok_twitgoo_PART_SRCS twitgoosettings.kcfgc )

add_library(choqok_twitgoo MODULE ${choqok_twitgoo_PART_SRCS})

target_link_libraries(choqok_twitgoo KF5::KIOCore ${QTOAUTH_LIBRARY} ${QJSON_LIBRARY} choqok twitterapihelper)

install(TARGETS choqok_twitgoo DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

set( kcm_choqok_twitgoo_PART_SRCS
twitgooconfig.cpp )

ki18n_wrap_ui(kcm_choqok_twitgoo_PART_SRCS twitgooprefs.ui )

kconfig_add_kcfg_files(kcm_choqok_twitgoo_PART_SRCS twitgoosettings.kcfgc )

add_library(kcm_choqok_twitgoo MODULE ${kcm_choqok_twitgoo_PART_SRCS})

target_link_libraries(kcm_choqok_twitgoo  choqok)

install(TARGETS kcm_choqok_twitgoo DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

install( FILES twitgoosettings.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES choqok_twitgoo_config.desktop choqok_twitgoo.desktop
         DESTINATION ${SERVICES_INSTALL_DIR})

add_subdirectory(icons)
