add_library(choqok_bit_ly MODULE
    bit_ly.cpp
)

kconfig_add_kcfg_files(choqok_bit_ly bit_ly_settings.kcfgc)

kcoreaddons_desktop_to_json(choqok_bit_ly choqok_bit_ly.desktop)

target_link_libraries(choqok_bit_ly
PUBLIC
    Qt::Core
    KF5::CoreAddons
    KF5::I18n
    KF5::KIOCore
    choqok
)

install(TARGETS choqok_bit_ly DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES choqok_bit_ly.desktop DESTINATION ${KDE_INSTALL_KSERVICESDIR})

add_library(kcm_choqok_bit_ly MODULE
    bit_ly_config.cpp
)

ki18n_wrap_ui(kcm_choqok_bit_ly bit_ly_prefs.ui)

kconfig_add_kcfg_files(kcm_choqok_bit_ly bit_ly_settings.kcfgc)

kcoreaddons_desktop_to_json(kcm_choqok_bit_ly choqok_bit_ly_config.desktop)

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

install(TARGETS kcm_choqok_bit_ly DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES bit_ly_settings.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
install(FILES choqok_bit_ly_config.desktop DESTINATION ${KDE_INSTALL_KSERVICESDIR})
