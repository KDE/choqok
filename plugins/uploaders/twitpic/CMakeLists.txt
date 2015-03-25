include_directories(
    ${CHOQOK_INCLUDES}
)

set(choqok_twitpic_PART_SRCS
    twitpic.cpp
)

# ki18n_wrap_ui(choqok_twitpic_PART_SRCS twitpicuploadimage_base.ui)

kconfig_add_kcfg_files(choqok_twitpic_PART_SRCS twitpicsettings.kcfgc)

add_library(choqok_twitpic MODULE ${choqok_twitpic_PART_SRCS})

kcoreaddons_desktop_to_json(choqok_twitpic choqok_twitpic.desktop)

target_link_libraries(choqok_twitpic
PUBLIC
    Qt5::Core
    KF5::CoreAddons
    KF5::I18n
    KF5::KIOCore
    ${QTOAUTH_LIBRARY}
    choqok
    twitterapihelper
)

install(TARGETS choqok_twitpic DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES choqok_twitpic.desktop DESTINATION ${SERVICES_INSTALL_DIR})

set(kcm_choqok_twitpic_PART_SRCS
twitpicconfig.cpp )

ki18n_wrap_ui(kcm_choqok_twitpic_PART_SRCS twitpicprefs.ui)

kconfig_add_kcfg_files(kcm_choqok_twitpic_PART_SRCS twitpicsettings.kcfgc )

add_library(kcm_choqok_twitpic MODULE ${kcm_choqok_twitpic_PART_SRCS})

kcoreaddons_desktop_to_json(kcm_choqok_twitpic choqok_twitpic_config.desktop)

target_link_libraries(kcm_choqok_twitpic choqok)

install(TARGETS kcm_choqok_twitpic DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES twitpicsettings.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install(FILES choqok_twitpic_config.desktop DESTINATION ${SERVICES_INSTALL_DIR})
# install( FILES twitpicui.rc  DESTINATION  ${DATA_INSTALL_DIR}/choqok_twitpic )

add_subdirectory(icons)