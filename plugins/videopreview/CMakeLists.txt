kcoreaddons_add_plugin(choqok_videopreview SOURCES videopreview.cpp INSTALL_NAMESPACE "choqok_plugins")

target_link_libraries(choqok_videopreview
PUBLIC
    Qt::Core
    KF5::CoreAddons
    KF5::KIOCore
    KF5::KIOWidgets
    choqok
)
