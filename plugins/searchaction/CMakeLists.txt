kcoreaddons_add_plugin(choqok_searchaction
    SOURCES
        searchaction.cpp
        searchaction.qrc
    INSTALL_NAMESPACE "choqok_plugins"
)

target_link_libraries(choqok_searchaction
PUBLIC
    Qt::Core
    Qt::Widgets
    KF5::CoreAddons
    KF5::I18n
    KF5::WidgetsAddons
    KF5::XmlGui
    choqok
    twitterapihelper
)
