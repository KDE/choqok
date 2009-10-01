
include_directories(
${CHOQOK_INCLUDES}
)

########### next target ###############

set( choqok_searchaction_PART_SRCS
searchaction.cpp )


kde4_add_plugin(choqok_searchaction ${choqok_searchaction_PART_SRCS})

target_link_libraries(choqok_searchaction ${KDE4_KIO_LIBS} choqok twitterapihelper)

install(TARGETS choqok_searchaction DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

install( FILES choqok_searchaction.desktop
         DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES searchactionui.rc  DESTINATION  ${DATA_INSTALL_DIR}/choqok )
