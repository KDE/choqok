
include_directories(
${CHOQOK_INCLUDES}
)

########### next target ###############

set( choqok_videopreview_PART_SRCS
videopreview.cpp )


add_library(choqok_videopreview MODULE ${choqok_videopreview_PART_SRCS})

target_link_libraries(choqok_videopreview KF5::KIOCore choqok)

install(TARGETS choqok_videopreview DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

install( FILES choqok_videopreview.desktop
         DESTINATION ${SERVICES_INSTALL_DIR})
