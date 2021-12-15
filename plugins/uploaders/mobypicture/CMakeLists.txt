set(choqok_mobypicture_PART_SRCS
    mobypicture.cpp
)

kconfig_add_kcfg_files(choqok_mobypicture_PART_SRCS mobypicturesettings.kcfgc)

add_library(choqok_mobypicture MODULE ${choqok_mobypicture_PART_SRCS})

kcoreaddons_desktop_to_json(choqok_mobypicture choqok_mobypicture.desktop)

target_link_libraries(choqok_mobypicture
PUBLIC
    Qt5::Core
    Qt5::NetworkAuth
    KF5::CoreAddons
    KF5::KIOCore
    choqok
    twitterapihelper
)

install(TARGETS choqok_mobypicture DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES choqok_mobypicture.desktop DESTINATION ${SERVICES_INSTALL_DIR})

set(kcm_choqok_mobypicture_PART_SRCS
    mobypictureconfig.cpp
)

ki18n_wrap_ui(kcm_choqok_mobypicture_PART_SRCS mobypictureprefs.ui)

kconfig_add_kcfg_files(kcm_choqok_mobypicture_PART_SRCS mobypicturesettings.kcfgc)

add_library(kcm_choqok_mobypicture MODULE ${kcm_choqok_mobypicture_PART_SRCS})

kcoreaddons_desktop_to_json(kcm_choqok_mobypicture choqok_mobypicture_config.desktop)

target_link_libraries(kcm_choqok_mobypicture
PUBLIC
    Qt5::Widgets
    KF5::ConfigWidgets
    KF5::CoreAddons
    KF5::I18n
    KF5::KIOCore
    KF5::WidgetsAddons
    choqok
)

install(TARGETS kcm_choqok_mobypicture DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES mobypicturesettings.kcfg DESTINATION ${KCFG_INSTALL_DIR})
install(FILES choqok_mobypicture_config.desktop DESTINATION ${SERVICES_INSTALL_DIR})

add_subdirectory(icons)
