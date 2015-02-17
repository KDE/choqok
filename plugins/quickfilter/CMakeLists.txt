include_directories(
${CHOQOK_INCLUDES}
)

########### next target ###############

set( choqok_quickfilter_PART_SRCS
quickfilter.cpp )


add_library(choqok_quickfilter MODULE ${choqok_quickfilter_PART_SRCS})

target_link_libraries(choqok_quickfilter KF5::KIOCore choqok)

install(TARGETS choqok_quickfilter DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

install( FILES choqok_quickfilter.desktop
         DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES quickfilterui.rc  DESTINATION  ${DATA_INSTALL_DIR}/choqok_quickfilter )