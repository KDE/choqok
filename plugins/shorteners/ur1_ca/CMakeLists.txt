include_directories(
${CHOQOK_INCLUDES}
)

set( choqok_ur1_ca_SRCS
ur1_ca.cpp )

kde4_add_plugin(choqok_ur1_ca ${choqok_ur1_ca_SRCS})

target_link_libraries(choqok_ur1_ca ${KDE4_KIO_LIBS} choqok)

install(TARGETS choqok_ur1_ca DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES choqok_ur1_ca.desktop  DESTINATION ${SERVICES_INSTALL_DIR})