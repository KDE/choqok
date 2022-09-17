kcoreaddons_add_plugin(choqok_ur1_ca SOURCES ur1_ca.cpp INSTALL_NAMESPACE "choqok_plugins")

target_link_libraries(choqok_ur1_ca
PUBLIC
    Qt::Core
    KF5::CoreAddons
    KF5::I18n
    KF5::KIOCore
    choqok
)
