
include_directories(
${CHOQOK_INCLUDES}
)

add_subdirectory(icons)
########### next target ###############

set( choqok_twitpic_PART_SRCS
twitpic.cpp)

# ki18n_wrap_ui(choqok_twitpic_PART_SRCS twitpicuploadimage_base.ui)

kconfig_add_kcfg_files(choqok_twitpic_PART_SRCS twitpicsettings.kcfgc )

add_library(choqok_twitpic MODULE ${choqok_twitpic_PART_SRCS})

target_link_libraries(choqok_twitpic KF5::KIOCore ${QTOAUTH_LIBRARY} ${QJSON_LIBRARY} choqok twitterapihelper)

install(TARGETS choqok_twitpic DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

set( kcm_choqok_twitpic_PART_SRCS
twitpicconfig.cpp )

ki18n_wrap_ui(kcm_choqok_twitpic_PART_SRCS twitpicprefs.ui )

kconfig_add_kcfg_files(kcm_choqok_twitpic_PART_SRCS twitpicsettings.kcfgc )

add_library(kcm_choqok_twitpic MODULE ${kcm_choqok_twitpic_PART_SRCS})

target_link_libraries(kcm_choqok_twitpic  choqok)

install(TARGETS kcm_choqok_twitpic DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

install( FILES twitpicsettings.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES choqok_twitpic_config.desktop choqok_twitpic.desktop
         DESTINATION ${SERVICES_INSTALL_DIR})
# install( FILES twitpicui.rc  DESTINATION  ${DATA_INSTALL_DIR}/choqok_twitpic )
