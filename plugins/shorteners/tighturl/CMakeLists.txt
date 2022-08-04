add_library(choqok_tighturl MODULE
    tighturl.cpp
)

kcoreaddons_desktop_to_json(choqok_tighturl choqok_tighturl.desktop)

target_link_libraries(choqok_tighturl
PUBLIC
    Qt::Core
    KF5::CoreAddons
    KF5::I18n
    KF5::KIOCore
    choqok
)

install(TARGETS choqok_tighturl DESTINATION ${KDE_INSTALL_PLUGINDIR})
install( FILES choqok_tighturl.desktop DESTINATION ${KDE_INSTALL_KSERVICESDIR})
