include_directories(
${CHOQOK_INCLUDES}
)

set( choqok_tighturl_SRCS
tighturl.cpp )

kde4_add_plugin(choqok_tighturl ${choqok_tighturl_SRCS})

target_link_libraries(choqok_tighturl ${KDE4_KIO_LIBS} choqok)

install(TARGETS choqok_tighturl DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES choqok_tighturl.desktop  DESTINATION ${SERVICES_INSTALL_DIR})