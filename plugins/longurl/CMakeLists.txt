add_library(choqok_longurl MODULE
    longurl.cpp
)

# kconfig_add_kcfg_files(choqok_longurl longurlsettings.kcfgc)

kcoreaddons_desktop_to_json(choqok_longurl choqok_longurl.desktop)

target_link_libraries(choqok_longurl
PUBLIC
    Qt5::Core
    KF5::CoreAddons
    KF5::KIOCore
    KF5::KIOWidgets
    choqok
)

install(TARGETS choqok_longurl DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES choqok_longurl.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
