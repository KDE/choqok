kcoreaddons_add_plugin(choqok_filter SOURCES
    addeditfilter.cpp
    configurefilters.cpp
    filter.cpp
    filtermanager.cpp
    filtersettings.cpp
    filter.qrc
    INSTALL_NAMESPACE "choqok_plugins"
)

ki18n_wrap_ui(choqok_filter addeditfilter_base.ui filterprefs.ui)

target_link_libraries(choqok_filter
PUBLIC
    Qt::Core
    Qt::Widgets
    KF5::ConfigCore
    KF5::CoreAddons
    KF5::I18n
    KF5::KIOCore
    KF5::WidgetsAddons
    KF5::XmlGui
    choqok
    twitterapihelper
)
