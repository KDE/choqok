add_library(choqok_betternotify MODULE
    mytextbrowser.cpp
    notification.cpp
    notify.cpp
    notifysettings.cpp
)

# kconfig_add_kcfg_files(choqok_betternotify betternotify.kcfgc)

kcoreaddons_desktop_to_json(choqok_betternotify choqok_notify.desktop)

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

install(TARGETS choqok_betternotify DESTINATION ${KDE_INSTALL_PLUGINDIR})

add_library(kcm_choqok_betternotify MODULE
    dummynotification.cpp
    notifyconfig.cpp
    notifysettings.cpp
)

ki18n_wrap_ui(kcm_choqok_betternotify notifyprefs.ui)

# kconfig_add_kcfg_files(kcm_choqok_betternotify betternotify.kcfgc)

kcoreaddons_desktop_to_json(kcm_choqok_betternotify choqok_notify_config.desktop)

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

install(TARGETS kcm_choqok_betternotify DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES choqok_notify.desktop choqok_notify_config.desktop
        DESTINATION ${KDE_INSTALL_KSERVICESDIR})
