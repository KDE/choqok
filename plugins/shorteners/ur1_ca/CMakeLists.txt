add_library(choqok_ur1_ca MODULE
    ur1_ca.cpp
)

kcoreaddons_desktop_to_json(choqok_ur1_ca choqok_ur1_ca.desktop)

target_link_libraries(choqok_ur1_ca
PUBLIC
    Qt5::Core
    KF5::CoreAddons
    KF5::I18n
    KF5::KIOCore
    choqok
)

install(TARGETS choqok_ur1_ca DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES choqok_ur1_ca.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
