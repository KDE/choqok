kcoreaddons_add_plugin(choqok_ur_ly SOURCES ur_ly.cpp INSTALL_NAMESPACE "choqok_plugins")

target_link_libraries(choqok_ur_ly
PUBLIC
    Qt::Core
    KF5::CoreAddons
    KF5::I18n
    KF5::KIOCore
    choqok
)
