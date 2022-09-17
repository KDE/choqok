kcoreaddons_add_plugin(choqok_imageshack SOURCES imageshack.cpp INSTALL_NAMESPACE "choqok_plugins")

target_link_libraries(choqok_imageshack
PUBLIC
    Qt::Core
    Qt::Xml
    KF5::CoreAddons
    KF5::I18n
    KF5::KIOCore
    choqok
)

add_subdirectory(icons)
