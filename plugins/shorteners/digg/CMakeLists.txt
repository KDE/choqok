include_directories(
${CHOQOK_INCLUDES}
)

set( choqok_digg_SRCS
digg.cpp )

kde4_add_plugin(choqok_digg ${choqok_digg_SRCS})

target_link_libraries(choqok_digg ${KDE4_KIO_LIBS} choqok)

install(TARGETS choqok_digg DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES choqok_digg.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
