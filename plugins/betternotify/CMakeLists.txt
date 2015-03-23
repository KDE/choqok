include_directories(
    ${CHOQOK_INCLUDES}
)

set(choqok_notify_PART_SRCS
    mytextbrowser.cpp
    notification.cpp
    notify.cpp
    notifysettings.cpp
)

# kconfig_add_kcfg_files(choqok_notify_PART_SRCS betternotify.kcfgc)

add_library(choqok_betternotify MODULE ${choqok_notify_PART_SRCS})

kcoreaddons_desktop_to_json(choqok_betternotify choqok_notify.desktop)

target_link_libraries(choqok_betternotify
PUBLIC
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    KF5::CoreAddons
    KF5::I18n
    KF5::KDELibs4Support
    choqok
)

install(TARGETS choqok_betternotify DESTINATION ${PLUGIN_INSTALL_DIR})

set(kcm_choqok_notify_PART_SRCS
    dummynotification.cpp
    notifyconfig.cpp
    notifysettings.cpp
)

ki18n_wrap_ui(kcm_choqok_notify_PART_SRCS notifyprefs.ui)

# kconfig_add_kcfg_files(kcm_choqok_notify_PART_SRCS betternotify.kcfgc)

add_library(kcm_choqok_betternotify MODULE ${kcm_choqok_notify_PART_SRCS})

kcoreaddons_desktop_to_json(kcm_choqok_betternotify choqok_notify_config.desktop)

target_link_libraries(kcm_choqok_betternotify
PUBLIC
    Qt5::Core
    Qt5::Gui
    KF5::KDELibs4Support
    choqok
)

install(TARGETS kcm_choqok_betternotify DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES choqok_notify.desktop choqok_notify_config.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
