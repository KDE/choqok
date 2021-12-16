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

install(TARGETS choqok_videopreview DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES choqok_videopreview.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
