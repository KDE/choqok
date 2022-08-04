add_library(choqok_imageshack MODULE
    imageshack.cpp
)

kcoreaddons_desktop_to_json(choqok_imageshack choqok_imageshack.desktop)

target_link_libraries(choqok_imageshack
PUBLIC
    Qt::Core
    Qt::Xml
    KF5::CoreAddons
    KF5::I18n
    KF5::KIOCore
    choqok
)

install(TARGETS choqok_imageshack DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES choqok_imageshack.desktop DESTINATION ${KDE_INSTALL_KSERVICESDIR})

add_subdirectory(icons)
