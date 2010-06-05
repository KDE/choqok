include_directories(
${CHOQOK_INCLUDES}
)

set( choqok_u_nu_SRCS
u_nu.cpp )

kde4_add_plugin(choqok_u_nu ${choqok_u_nu_SRCS})

target_link_libraries(choqok_u_nu ${KDE4_KIO_LIBS} choqok)

install(TARGETS choqok_u_nu DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES choqok_u_nu.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
 
