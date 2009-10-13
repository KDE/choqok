include_directories(
${CHOQOK_INCLUDES}
)

set( choqok_is_gd_SRCS
is_gd.cpp )

kde4_add_plugin(choqok_is_gd ${choqok_is_gd_SRCS})

target_link_libraries(choqok_is_gd ${KDE4_KIO_LIBS} choqok)

install(TARGETS choqok_is_gd DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES choqok_is_gd.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
 
