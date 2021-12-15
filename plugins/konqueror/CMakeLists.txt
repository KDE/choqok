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

install(TARGETS konqchoqokplugin DESTINATION ${PLUGIN_INSTALL_DIR}/kf5/parts)
install(FILES konqchoqok.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/konqchoqokplugin)
install(FILES konqchoqok.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES choqok_share.desktop DESTINATION ${SERVICES_INSTALL_DIR}/ServiceMenus)
