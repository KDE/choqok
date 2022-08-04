find_package(TelepathyQt${QT_MAJOR_VERSION})

if(TelepathyQt${QT_MAJOR_VERSION}_FOUND)
    add_definitions( -DTELEPATHY_FOUND )
    # TelepathyQt5 headers use Q_FOREACH
    remove_definitions(-DQT_NO_FOREACH)
    include_directories( SYSTEM ${TELEPATHY_QT${QT_MAJOR_VERSION}_INCLUDE_DIR})
endif()

add_library(choqok_imstatus MODULE
    imqdbus.cpp
    imstatus.cpp
)

kconfig_add_kcfg_files(choqok_imstatus imstatussettings.kcfgc)

kcoreaddons_desktop_to_json(choqok_imstatus choqok_imstatus.desktop)

target_link_libraries(choqok_imstatus
PUBLIC
    Qt::Core
    Qt::DBus
    KF5::CoreAddons
    ${TELEPATHY_QT5_LIBRARIES}
    choqok
)

install(TARGETS choqok_imstatus DESTINATION ${KDE_INSTALL_PLUGINDIR})

add_library(kcm_choqok_imstatus MODULE
    imqdbus.cpp
    imstatusconfig.cpp
)

ki18n_wrap_ui(kcm_choqok_imstatus imstatusprefs.ui)

kconfig_add_kcfg_files(kcm_choqok_imstatus imstatussettings.kcfgc)

kcoreaddons_desktop_to_json(kcm_choqok_imstatus choqok_imstatus_config.desktop)

target_link_libraries(kcm_choqok_imstatus
PUBLIC
    Qt::Core
    Qt::DBus
    KF5::ConfigWidgets
    KF5::CoreAddons
    KF5::I18n
    ${TELEPATHY_QT5_LIBRARIES}
)

install(TARGETS kcm_choqok_imstatus DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES imstatussettings.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
install(FILES choqok_imstatus_config.desktop choqok_imstatus.desktop
        DESTINATION ${KDE_INSTALL_KSERVICESDIR})
