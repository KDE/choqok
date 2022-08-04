add_library(choqok_goo_gl MODULE
    goo_gl.cpp
)

kcoreaddons_desktop_to_json(choqok_goo_gl choqok_goo_gl.desktop)

target_link_libraries(choqok_goo_gl
PUBLIC
    Qt::Core
    KF5::CoreAddons
    KF5::I18n
    KF5::KIOCore
    choqok
)

install(TARGETS choqok_goo_gl DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES choqok_goo_gl.desktop DESTINATION ${KDE_INSTALL_KSERVICESDIR})
