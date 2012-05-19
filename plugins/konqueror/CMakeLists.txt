#include(DBusMacros)

include_directories(
${CHOQOK_INCLUDES}
)

set(konqchoqokplugin_SRCS konqchoqok.cpp)

kde4_add_plugin(konqchoqokplugin ${konqchoqokplugin_SRCS})

target_link_libraries(konqchoqokplugin ${KDE4_KHTML_LIBS} choqok )

install(TARGETS konqchoqokplugin DESTINATION ${PLUGIN_INSTALL_DIR} )

install(FILES konqchoqok.rc konqchoqok.desktop DESTINATION ${DATA_INSTALL_DIR}/khtml/kpartplugins )

install( FILES choqok_share.desktop DESTINATION ${SERVICES_INSTALL_DIR}/ServiceMenus )
install( FILES choqok_upload.desktop DESTINATION ${SERVICES_INSTALL_DIR}/ServiceMenus )
