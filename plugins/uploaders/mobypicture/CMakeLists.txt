kcoreaddons_add_plugin(choqok_mobypicture SOURCES mobypicture.cpp INSTALL_NAMESPACE "choqok_plugins")

kconfig_add_kcfg_files(choqok_mobypicture mobypicturesettings.kcfgc)

target_link_libraries(choqok_mobypicture
PUBLIC
    Qt::Core
    Qt::NetworkAuth
    KF5::CoreAddons
    KF5::KIOCore
    choqok
    twitterapihelper
)

kcoreaddons_add_plugin(kcm_choqok_mobypicture SOURCES mobypictureconfig.cpp INSTALL_NAMESPACE "choqok_plugins/kcms")

ki18n_wrap_ui(kcm_choqok_mobypicture mobypictureprefs.ui)

kconfig_add_kcfg_files(kcm_choqok_mobypicture mobypicturesettings.kcfgc)

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

add_subdirectory(icons)
