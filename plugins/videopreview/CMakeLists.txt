add_library(choqok_videopreview MODULE
    videopreview.cpp
)

kcoreaddons_desktop_to_json(choqok_videopreview choqok_videopreview.desktop)

target_link_libraries(choqok_videopreview
PUBLIC
    Qt5::Core
    KF5::CoreAddons
    KF5::KIOCore
    KF5::KIOWidgets
    choqok
)

install(TARGETS choqok_videopreview DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES choqok_videopreview.desktop DESTINATION ${SERVICES_INSTALL_DIR})
