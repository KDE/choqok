kcoreaddons_add_plugin(choqok_quickfilter
    SOURCES
        quickfilter.cpp
        quickfilter.qrc
    INSTALL_NAMESPACE "choqok_plugins"
)

target_link_libraries(choqok_quickfilter
PUBLIC
    Qt::Core
    Qt::Widgets
    KF5::CoreAddons
    KF5::I18n
    KF5::XmlGui
    choqok
)
