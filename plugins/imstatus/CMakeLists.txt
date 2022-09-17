find_package(TelepathyQt${QT_MAJOR_VERSION})

if(TelepathyQt${QT_MAJOR_VERSION}_FOUND)
    add_definitions( -DTELEPATHY_FOUND )
    # TelepathyQt5 headers use Q_FOREACH
    remove_definitions(-DQT_NO_FOREACH)
    include_directories( SYSTEM ${TELEPATHY_QT${QT_MAJOR_VERSION}_INCLUDE_DIR})
endif()

kcoreaddons_add_plugin(choqok_imstatus SOURCES imqdbus.cpp imstatus.cpp INSTALL_NAMESPACE "choqok_plugins")

kconfig_add_kcfg_files(choqok_imstatus imstatussettings.kcfgc)

target_link_libraries(choqok_imstatus
PUBLIC
    Qt::Core
    Qt::DBus
    KF5::CoreAddons
    ${TELEPATHY_QT5_LIBRARIES}
    choqok
)

kcoreaddons_add_plugin(kcm_choqok_imstatus SOURCES imqdbus.cpp imstatusconfig.cpp INSTALL_NAMESPACE "choqok_plugins/kcms")

ki18n_wrap_ui(kcm_choqok_imstatus imstatusprefs.ui)

kconfig_add_kcfg_files(kcm_choqok_imstatus imstatussettings.kcfgc)

target_link_libraries(kcm_choqok_imstatus
PUBLIC
    Qt::Core
    Qt::DBus
    KF5::ConfigWidgets
    KF5::CoreAddons
    KF5::I18n
    ${TELEPATHY_QT5_LIBRARIES}
)
