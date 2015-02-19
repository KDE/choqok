
include_directories(
${CHOQOK_INCLUDES}
)

set( choqok_imagepreview_PART_SRCS
imagepreview.cpp )

add_library(choqok_imagepreview MODULE ${choqok_imagepreview_PART_SRCS})

kcoreaddons_desktop_to_json(choqok_imagepreview choqok_imagepreview.desktop)

target_link_libraries(choqok_imagepreview
PUBLIC
    Qt5::Core
    Qt5::Gui
    KF5::CoreAddons
    choqok
)

install(TARGETS choqok_imagepreview DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

install( FILES choqok_imagepreview.desktop
         DESTINATION ${SERVICES_INSTALL_DIR})
