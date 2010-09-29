include_directories(
${CHOQOK_INCLUDES}
)

set( choqok_goo_gl_SRCS
goo_gl.cpp )

kde4_add_plugin(choqok_goo_gl ${choqok_goo_gl_SRCS})

target_link_libraries(choqok_goo_gl ${KDE4_KIO_LIBS} ${QJSON_LIBRARY} choqok)

install(TARGETS choqok_goo_gl DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES choqok_goo_gl.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
 
