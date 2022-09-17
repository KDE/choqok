kcoreaddons_add_plugin(choqok_posterous SOURCES posterous.cpp INSTALL_NAMESPACE "choqok_plugins")

kconfig_add_kcfg_files(choqok_posterous posteroussettings.kcfgc)

target_link_libraries(choqok_posterous
PUBLIC
    Qt::Core
    Qt::NetworkAuth
    KF5::CoreAddons
    KF5::KIOCore
    choqok
    twitterapihelper
)

kcoreaddons_add_plugin(kcm_choqok_posterous SOURCES posterousconfig.cpp INSTALL_NAMESPACE "choqok_plugins/kcms")

ki18n_wrap_ui(kcm_choqok_posterous posterousprefs.ui)

kconfig_add_kcfg_files(kcm_choqok_posterous posteroussettings.kcfgc)

target_link_libraries(kcm_choqok_posterous
PUBLIC
    Qt::Widgets
    KF5::ConfigWidgets
    KF5::CoreAddons
    KF5::I18n
    KF5::WidgetsAddons
    choqok
)

add_subdirectory(icons)
