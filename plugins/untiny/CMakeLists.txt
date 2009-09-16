
include_directories(
${CHOQOK_INCLUDES}
)

########### next target ###############

set( choqok_untiny_PART_SRCS
untiny.cpp )


kde4_add_plugin(choqok_untiny ${choqok_untiny_PART_SRCS})

target_link_libraries(choqok_untiny ${KDE4_KIO_LIBS} choqok)

install(TARGETS choqok_untiny DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

install( FILES choqok_untiny.desktop
         DESTINATION ${SERVICES_INSTALL_DIR})
