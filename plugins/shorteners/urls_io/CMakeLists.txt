include_directories(
${CHOQOK_INCLUDES}
)

set( choqok_urls_io_SRCS
urls_io.cpp )

kde4_add_plugin(choqok_urls_io ${choqok_urls_io_SRCS})

target_link_libraries(choqok_urls_io ${KDE4_KIO_LIBS} choqok)

install(TARGETS choqok_urls_io DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES choqok_urls_io.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
