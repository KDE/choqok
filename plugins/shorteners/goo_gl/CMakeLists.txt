kcoreaddons_add_plugin(choqok_goo_gl SOURCES goo_gl.cpp INSTALL_NAMESPACE "choqok_plugins")

target_link_libraries(choqok_goo_gl
PUBLIC
    Qt::Core
    KF5::CoreAddons
    KF5::I18n
    KF5::KIOCore
    choqok
)
