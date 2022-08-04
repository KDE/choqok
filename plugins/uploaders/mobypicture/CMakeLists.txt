add_library(choqok_mobypicture MODULE
    mobypicture.cpp
)

kconfig_add_kcfg_files(choqok_mobypicture mobypicturesettings.kcfgc)

kcoreaddons_desktop_to_json(choqok_mobypicture choqok_mobypicture.desktop)

target_link_libraries(choqok_mobypicture
PUBLIC
    Qt::Core
    Qt::NetworkAuth
    KF5::CoreAddons
    KF5::KIOCore
    choqok
    twitterapihelper
)

install(TARGETS choqok_mobypicture DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES choqok_mobypicture.desktop DESTINATION ${KDE_INSTALL_KSERVICESDIR})

add_library(kcm_choqok_mobypicture MODULE
    mobypictureconfig.cpp
)

ki18n_wrap_ui(kcm_choqok_mobypicture mobypictureprefs.ui)

kconfig_add_kcfg_files(kcm_choqok_mobypicture mobypicturesettings.kcfgc)

kcoreaddons_desktop_to_json(kcm_choqok_mobypicture choqok_mobypicture_config.desktop)

target_link_libraries(kcm_choqok_mobypicture
PUBLIC
    Qt::Widgets
    KF5::ConfigWidgets
    KF5::CoreAddons
    KF5::I18n
    KF5::KIOCore
    KF5::WidgetsAddons
    choqok
)

install(TARGETS kcm_choqok_mobypicture DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES mobypicturesettings.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
install(FILES choqok_mobypicture_config.desktop DESTINATION ${KDE_INSTALL_KSERVICESDIR})

add_subdirectory(icons)
