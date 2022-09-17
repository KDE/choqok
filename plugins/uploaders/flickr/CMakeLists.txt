kcoreaddons_add_plugin(choqok_flickr SOURCES flickr.cpp INSTALL_NAMESPACE "choqok_plugins")

kconfig_add_kcfg_files(choqok_flickr flickrsettings.kcfgc)

target_link_libraries(choqok_flickr
PUBLIC
    Qt::Core
    Qt::Xml
    KF5::ConfigWidgets
    KF5::CoreAddons
    KF5::KIOCore
    choqok
)

kcoreaddons_add_plugin(kcm_choqok_flickr SOURCES flickrconfig.cpp INSTALL_NAMESPACE "choqok_plugins/kcms")

ki18n_wrap_ui(kcm_choqok_flickr flickrprefs.ui)

kconfig_add_kcfg_files(kcm_choqok_flickr flickrsettings.kcfgc)

target_link_libraries(kcm_choqok_flickr
PUBLIC
    Qt::Core
    Qt::Widgets
    Qt::Xml
    KF5::ConfigCore
    KF5::CoreAddons
    KF5::I18n
    KF5::KIOCore
    KF5::WidgetsAddons
    choqok
)

add_subdirectory(icons)
