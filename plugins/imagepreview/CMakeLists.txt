
include_directories(
${CHOQOK_INCLUDES}
)

########### next target ###############

set( choqok_imagepreview_PART_SRCS
imagepreview.cpp )


add_library(choqok_imagepreview MODULE ${choqok_imagepreview_PART_SRCS})

target_link_libraries(choqok_imagepreview KF5::KIOCore choqok)

install(TARGETS choqok_imagepreview DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

install( FILES choqok_imagepreview.desktop
         DESTINATION ${SERVICES_INSTALL_DIR})
