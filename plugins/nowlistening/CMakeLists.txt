add_library(choqok_nowlistening MODULE
    mpris.cpp
    nowlistening.cpp
)

kconfig_add_kcfg_files(choqok_nowlistening nowlisteningsettings.kcfgc)

kcoreaddons_desktop_to_json(choqok_nowlistening choqok_nowlistening.desktop)

target_link_libraries(choqok_nowlistening
PUBLIC
    Qt::Core
    Qt::DBus
    Qt::Widgets
    KF5::CoreAddons
    KF5::I18n
    KF5::WidgetsAddons
    KF5::XmlGui
    choqok
)

install(TARGETS choqok_nowlistening DESTINATION ${KDE_INSTALL_PLUGINDIR})

add_library(kcm_choqok_nowlistening MODULE
    nowlisteningconfig.cpp
)

ki18n_wrap_ui(kcm_choqok_nowlistening nowlisteningprefs.ui)

kconfig_add_kcfg_files(kcm_choqok_nowlistening nowlisteningsettings.kcfgc)

kcoreaddons_desktop_to_json(kcm_choqok_nowlistening choqok_nowlistening_config.desktop)

target_link_libraries(kcm_choqok_nowlistening
PUBLIC
    Qt::Widgets
    KF5::ConfigWidgets
    KF5::CoreAddons
    KF5::I18n
)

install(TARGETS kcm_choqok_nowlistening DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES nowlisteningsettings.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
install(FILES choqok_nowlistening_config.desktop choqok_nowlistening.desktop
        DESTINATION ${KDE_INSTALL_KSERVICESDIR})
install(FILES nowlisteningui.rc DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/choqok_nowlistening)
