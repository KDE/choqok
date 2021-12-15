set(choqok_posterous_PART_SRCS
    posterous.cpp
)

kconfig_add_kcfg_files(choqok_posterous_PART_SRCS posteroussettings.kcfgc)

add_library(choqok_posterous MODULE ${choqok_posterous_PART_SRCS})

kcoreaddons_desktop_to_json(choqok_posterous choqok_posterous.desktop)

target_link_libraries(choqok_posterous
PUBLIC
    Qt5::Core
    Qt5::NetworkAuth
    KF5::CoreAddons
    KF5::KIOCore
    choqok
    twitterapihelper
)

install(TARGETS choqok_posterous DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES choqok_posterous.desktop DESTINATION ${SERVICES_INSTALL_DIR})

set(kcm_choqok_posterous_PART_SRCS
    posterousconfig.cpp
)

ki18n_wrap_ui(kcm_choqok_posterous_PART_SRCS posterousprefs.ui)

kconfig_add_kcfg_files(kcm_choqok_posterous_PART_SRCS posteroussettings.kcfgc)

add_library(kcm_choqok_posterous MODULE ${kcm_choqok_posterous_PART_SRCS})

kcoreaddons_desktop_to_json(kcm_choqok_posterous choqok_posterous_config.desktop)

target_link_libraries(kcm_choqok_posterous
PUBLIC
    Qt5::Widgets
    KF5::ConfigWidgets
    KF5::CoreAddons
    KF5::I18n
    KF5::WidgetsAddons
    choqok
)

install(TARGETS kcm_choqok_posterous DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES posteroussettings.kcfg DESTINATION ${KCFG_INSTALL_DIR})
install(FILES choqok_posterous_config.desktop DESTINATION ${SERVICES_INSTALL_DIR})

add_subdirectory(icons)
