add_library(choqok_posterous MODULE
    posterous.cpp
)

kconfig_add_kcfg_files(choqok_posterous posteroussettings.kcfgc)

kcoreaddons_desktop_to_json(choqok_posterous choqok_posterous.desktop)

target_link_libraries(choqok_posterous
PUBLIC
    Qt::Core
    Qt::NetworkAuth
    KF5::CoreAddons
    KF5::KIOCore
    choqok
    twitterapihelper
)

install(TARGETS choqok_posterous DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES choqok_posterous.desktop DESTINATION ${KDE_INSTALL_KSERVICESDIR})

add_library(kcm_choqok_posterous MODULE
    posterousconfig.cpp
)

ki18n_wrap_ui(kcm_choqok_posterous posterousprefs.ui)

kconfig_add_kcfg_files(kcm_choqok_posterous posteroussettings.kcfgc)

kcoreaddons_desktop_to_json(kcm_choqok_posterous choqok_posterous_config.desktop)

target_link_libraries(kcm_choqok_posterous
PUBLIC
    Qt::Widgets
    KF5::ConfigWidgets
    KF5::CoreAddons
    KF5::I18n
    KF5::WidgetsAddons
    choqok
)

install(TARGETS kcm_choqok_posterous DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES posteroussettings.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
install(FILES choqok_posterous_config.desktop DESTINATION ${KDE_INSTALL_KSERVICESDIR})

add_subdirectory(icons)
