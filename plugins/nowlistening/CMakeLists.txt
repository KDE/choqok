kcoreaddons_add_plugin(choqok_nowlistening SOURCES mpris.cpp nowlistening.cpp INSTALL_NAMESPACE "choqok_plugins")

kconfig_add_kcfg_files(choqok_nowlistening nowlisteningsettings.kcfgc)

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

kcoreaddons_add_plugin(kcm_choqok_nowlistening SOURCES nowlisteningconfig.cpp INSTALL_NAMESPACE "choqok_plugins/kcms")

ki18n_wrap_ui(kcm_choqok_nowlistening nowlisteningprefs.ui)

kconfig_add_kcfg_files(kcm_choqok_nowlistening nowlisteningsettings.kcfgc)

target_link_libraries(kcm_choqok_nowlistening
PUBLIC
    Qt::Widgets
    KF5::ConfigWidgets
    KF5::CoreAddons
    KF5::I18n
)

install(FILES nowlisteningui.rc DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/choqok_nowlistening)
