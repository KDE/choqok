find_package(QJSON REQUIRED)

include_directories(
	${CHOQOK_INCLUDES}
	${QJSON_INCLUDE_DIR}
)

set( choqok_longurl_PART_SRCS
longurl.cpp longurl.h)

# kde4_add_kcfg_files(choqok_longurl_PART_SRCS longurlsettings.kcfgc)

kde4_add_plugin(choqok_longurl ${choqok_longurl_PART_SRCS})

target_link_libraries(choqok_longurl ${KDE4_KIO_LIBS} ${QJSON_LIBRARIES} choqok)

install(TARGETS choqok_longurl DESTINATION ${PLUGIN_INSTALL_DIR})
install( FILES choqok_longurl.desktop DESTINATION ${SERVICES_INSTALL_DIR})

