include_directories(
${CHOQOK_INCLUDES}
)

set( choqok_goo_gl_SRCS
goo_gl.cpp )

add_library(choqok_goo_gl MODULE ${choqok_goo_gl_SRCS})

target_link_libraries(choqok_goo_gl KF5::KIOCore ${QJSON_LIBRARY} choqok)

install(TARGETS choqok_goo_gl DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES choqok_goo_gl.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
 
