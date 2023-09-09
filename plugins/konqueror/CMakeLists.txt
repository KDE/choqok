kcoreaddons_add_plugin(konqchoqokplugin
    SOURCES
        konqchoqok.cpp
        konqchoqok.qrc
    INSTALL_NAMESPACE "kf5/parts"
)

target_link_libraries(konqchoqokplugin
PUBLIC
    Qt::DBus
    Qt::Widgets
    KF5::CoreAddons
    KF5::I18n
    KF5::Parts
    KF5::Service
    KF5::WebKit
    KF5::WidgetsAddons
    KF5::XmlGui
)
