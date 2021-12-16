add_library(choqok_ur_ly MODULE
    ur_ly.cpp
)

kcoreaddons_desktop_to_json(choqok_ur_ly choqok_ur_ly.desktop)

target_link_libraries(choqok_ur_ly
PUBLIC
    Qt5::Core
    KF5::CoreAddons
    KF5::I18n
    KF5::KIOCore
    choqok
)

install(TARGETS choqok_ur_ly DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES choqok_ur_ly.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
