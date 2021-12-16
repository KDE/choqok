add_library(choqok_imageshack MODULE
    imageshack.cpp
)

kcoreaddons_desktop_to_json(choqok_imageshack choqok_imageshack.desktop)

target_link_libraries(choqok_imageshack
PUBLIC
    Qt5::Core
    Qt5::Xml
    KF5::CoreAddons
    KF5::I18n
    KF5::KIOCore
    choqok
)

install(TARGETS choqok_imageshack DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES choqok_imageshack.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})

add_subdirectory(icons)
