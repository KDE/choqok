
include_directories(
${CHOQOK_INCLUDES}
)

########### next target ###############

set( choqok_searchaction_PART_SRCS
searchaction.cpp )


add_library(choqok_searchaction MODULE ${choqok_searchaction_PART_SRCS})

target_link_libraries(choqok_searchaction KF5::KIOCore choqok twitterapihelper)

install(TARGETS choqok_searchaction DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

install( FILES choqok_searchaction.desktop
         DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES searchactionui.rc  DESTINATION  ${DATA_INSTALL_DIR}/choqok_searchaction )
