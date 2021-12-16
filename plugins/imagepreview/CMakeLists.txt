add_library(choqok_imagepreview MODULE
    imagepreview.cpp
)

kcoreaddons_desktop_to_json(choqok_imagepreview choqok_imagepreview.desktop)

target_link_libraries(choqok_imagepreview
PUBLIC
    Qt5::Core
    Qt5::Gui
    KF5::CoreAddons
    choqok
)

install(TARGETS choqok_imagepreview DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES choqok_imagepreview.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
