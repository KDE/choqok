include_directories(
${CHOQOK_INCLUDES}
${QJSON_INCLUDE_DIR}
)

set( choqok_ur_ly_SRCS
ur_ly.cpp )

kde4_add_plugin(choqok_ur_ly ${choqok_ur_ly_SRCS})

target_link_libraries(choqok_ur_ly ${KDE4_KIO_LIBS} ${QJSON_LIBRARY} choqok)

install(TARGETS choqok_ur_ly DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES choqok_ur_ly.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
