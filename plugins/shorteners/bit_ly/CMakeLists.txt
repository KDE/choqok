kcoreaddons_add_plugin(choqok_bit_ly SOURCES bit_ly.cpp INSTALL_NAMESPACE "choqok_plugins")

kconfig_add_kcfg_files(choqok_bit_ly bit_ly_settings.kcfgc)

target_link_libraries(choqok_bit_ly
PUBLIC
    Qt::Core
    KF5::CoreAddons
    KF5::I18n
    KF5::KIOCore
    choqok
)

kcoreaddons_add_plugin(kcm_choqok_bit_ly SOURCES bit_ly_config.cpp INSTALL_NAMESPACE "choqok_plugins/kcms")

ki18n_wrap_ui(kcm_choqok_bit_ly bit_ly_prefs.ui)

kconfig_add_kcfg_files(kcm_choqok_bit_ly bit_ly_settings.kcfgc)

target_link_libraries(kcm_choqok_bit_ly
PUBLIC
    Qt::Core
    KF5::ConfigCore
    KF5::ConfigWidgets
    KF5::CoreAddons
    KF5::I18n
    KF5::KIOCore
    KF5::WidgetsAddons
    choqok
)
