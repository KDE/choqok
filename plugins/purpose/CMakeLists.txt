kcoreaddons_add_plugin(choqokplugin SOURCES purposeplugin.cpp INSTALL_NAMESPACE "kf5/purpose")

set(choqokplugin_dbus_SRCS)
qt_add_dbus_interface(choqokplugin_dbus_SRCS org.kde.choqok.xml choqokinterface)
target_sources(choqokplugin PRIVATE ${choqokplugin_dbus_SRCS})

target_link_libraries(choqokplugin Qt::DBus KF5::Purpose)
