kcoreaddons_add_plugin(choqok_betternotify SOURCES mytextbrowser.cpp notification.cpp notify.cpp notifysettings.cpp INSTALL_NAMESPACE "choqok_plugins")

# kconfig_add_kcfg_files(choqok_betternotify betternotify.kcfgc)

target_link_libraries(choqok_betternotify
PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Widgets
    KF5::ConfigCore
    KF5::CoreAddons
    KF5::I18n
    choqok
)

kcoreaddons_add_plugin(kcm_choqok_betternotify SOURCES dummynotification.cpp notifyconfig.cpp notifysettings.cpp INSTALL_NAMESPACE "choqok_plugins/kcms")

ki18n_wrap_ui(kcm_choqok_betternotify notifyprefs.ui)

target_link_libraries(kcm_choqok_betternotify
PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Widgets
    KF5::ConfigCore
    KF5::ConfigWidgets
    KF5::I18n
    choqok
)
