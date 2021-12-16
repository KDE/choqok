add_library(choqok_flickr MODULE
    flickr.cpp
)

kconfig_add_kcfg_files(choqok_flickr flickrsettings.kcfgc)

kcoreaddons_desktop_to_json(choqok_flickr choqok_flickr.desktop)

target_link_libraries(choqok_flickr
PUBLIC
    Qt5::Core
    Qt5::Xml
    KF5::ConfigWidgets
    KF5::CoreAddons
    KF5::KIOCore
    choqok
)

install(TARGETS choqok_flickr DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES choqok_flickr.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})

add_library(kcm_choqok_flickr MODULE
    flickrconfig.cpp
)

ki18n_wrap_ui(kcm_choqok_flickr flickrprefs.ui)

kconfig_add_kcfg_files(kcm_choqok_flickr flickrsettings.kcfgc)

kcoreaddons_desktop_to_json(kcm_choqok_flickr choqok_flickr_config.desktop)

target_link_libraries(kcm_choqok_flickr
PUBLIC
    Qt5::Core
    Qt5::Widgets
    Qt5::Xml
    KF5::ConfigCore
    KF5::CoreAddons
    KF5::I18n
    KF5::KIOCore
    KF5::WidgetsAddons
    choqok
)

install(TARGETS kcm_choqok_flickr DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES flickrsettings.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
install(FILES choqok_flickr_config.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})

add_subdirectory(icons)
