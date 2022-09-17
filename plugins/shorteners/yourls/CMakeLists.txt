kcoreaddons_add_plugin(choqok_yourls SOURCES yourls.cpp INSTALL_NAMESPACE "choqok_plugins")

kconfig_add_kcfg_files(choqok_yourls yourlssettings.kcfgc)

target_link_libraries(choqok_yourls
PUBLIC
    Qt::Core
    KF5::CoreAddons
    KF5::I18n
    KF5::KIOCore
    choqok
)

kcoreaddons_add_plugin(kcm_choqok_yourls SOURCES yourlsconfig.cpp INSTALL_NAMESPACE "choqok_plugins/kcms")

ki18n_wrap_ui(kcm_choqok_yourls yourlsprefs.ui)

kconfig_add_kcfg_files(kcm_choqok_yourls yourlssettings.kcfgc)

target_link_libraries(kcm_choqok_yourls
PUBLIC
    Qt::Core
    KF5::ConfigWidgets
    KF5::CoreAddons
    KF5::I18n
    choqok
)
