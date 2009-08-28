
include_directories(
${CHOQOK_INCLUDES}
)

set( choqok_nowlistening_SRCS
nowlistening.cpp )

kde4_add_plugin(choqok_nowlistening ${choqok_nowlistening_SRCS})

target_link_libraries(choqok_nowlistening ${KDE4_KIO_LIBS} choqok)

install(TARGETS choqok_nowlistening DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES choqok_nowlistening.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES nowlisteningui.rc  DESTINATION  ${DATA_INSTALL_DIR}/choqok_nowlistening )