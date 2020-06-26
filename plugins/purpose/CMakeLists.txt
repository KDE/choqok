set(choqokplugin_SRCS purposeplugin.cpp)

qt5_add_dbus_interface(choqokplugin_SRCS org.kde.choqok.xml choqokinterface)

add_library(choqokplugin MODULE ${choqokplugin_SRCS})

target_link_libraries(choqokplugin Qt5::DBus KF5::Purpose)

install(TARGETS choqokplugin DESTINATION ${PLUGIN_INSTALL_DIR}/kf5/purpose)
