set(purposeplugin_SRCS purposeplugin.cpp)

qt5_add_dbus_interface(purposeplugin_SRCS org.kde.choqok.xml choqokinterface)

add_library(purposeplugin MODULE ${purposeplugin_SRCS})

target_link_libraries(purposeplugin Qt5::DBus KF5::Purpose)

install(TARGETS purposeplugin DESTINATION ${PLUGIN_INSTALL_DIR}/kf5/purpose)
