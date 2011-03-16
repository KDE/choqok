include_directories(
${CHOQOK_INCLUDES}
)

########### next target ###############

set( choqok_quickfilter_PART_SRCS
quickfilter.cpp )


kde4_add_plugin(choqok_quickfilter ${choqok_quickfilter_PART_SRCS})

target_link_libraries(choqok_quickfilter ${KDE4_KIO_LIBS} choqok)

install(TARGETS choqok_quickfilter DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

install( FILES choqok_quickfilter.desktop
         DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES quickfilterui.rc  DESTINATION  ${DATA_INSTALL_DIR}/choqok_quickfilter )