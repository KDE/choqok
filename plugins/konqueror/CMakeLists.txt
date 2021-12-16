add_library(konqchoqokplugin MODULE
    konqchoqok.cpp
)

kcoreaddons_desktop_to_json(konqchoqokplugin konqchoqok.desktop)

target_link_libraries(konqchoqokplugin
PUBLIC
    Qt5::DBus
    Qt5::Widgets
    KF5::CoreAddons
    KF5::I18n
    KF5::Parts
    KF5::Service
    KF5::WebKit
    KF5::WidgetsAddons
    KF5::XmlGui
)

install(TARGETS konqchoqokplugin DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf5/parts)
install(FILES konqchoqok.rc DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/konqchoqokplugin)
install(FILES konqchoqok.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install(FILES choqok_share.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR}/ServiceMenus)
