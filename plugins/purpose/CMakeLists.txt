add_library(choqokplugin MODULE
    purposeplugin.cpp
)

set(choqokplugin_dbus_SRCS)
qt5_add_dbus_interface(choqokplugin_dbus_SRCS org.kde.choqok.xml choqokinterface)
target_sources(choqokplugin PRIVATE ${choqokplugin_dbus_SRCS})

target_link_libraries(choqokplugin Qt5::DBus KF5::Purpose)

install(TARGETS choqokplugin DESTINATION ${PLUGIN_INSTALL_DIR}/kf5/purpose)
