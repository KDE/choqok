include_directories(
${CHOQOK_INCLUDES}
)

set( choqok_three_ly_SRCS
three_ly.cpp )

kde4_add_plugin(choqok_three_ly ${choqok_three_ly_SRCS})

target_link_libraries(choqok_three_ly ${KDE4_KIO_LIBS} choqok)

install(TARGETS choqok_three_ly DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES choqok_three_ly.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
 
