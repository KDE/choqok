kcoreaddons_add_plugin(choqok_longurl SOURCES longurl.cpp INSTALL_NAMESPACE "choqok_plugins")

# kconfig_add_kcfg_files(choqok_longurl longurlsettings.kcfgc)

target_link_libraries(choqok_longurl
PUBLIC
    Qt::Core
    KF5::CoreAddons
    KF5::KIOCore
    KF5::KIOWidgets
    choqok
)
