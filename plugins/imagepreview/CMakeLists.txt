kcoreaddons_add_plugin(choqok_imagepreview SOURCES imagepreview.cpp INSTALL_NAMESPACE "choqok_plugins")

target_link_libraries(choqok_imagepreview
PUBLIC
    Qt::Core
    Qt::Gui
    KF5::CoreAddons
    choqok
)
