
include_directories(
${CHOQOK_INCLUDES}
)

set( choqok_isgd_SRCS
is_gd.cpp )

kde4_add_plugin(choqok_isgd ${choqok_isgd_SRCS})

target_link_libraries(choqok_isgd ${KDE4_KIO_LIBS} choqok)

install(TARGETS choqok_isgd DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES choqok_isgd.desktop  DESTINATION ${SERVICES_INSTALL_DIR})