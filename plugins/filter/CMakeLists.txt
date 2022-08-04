add_library(choqok_filter MODULE
    addeditfilter.cpp
    configurefilters.cpp
    filter.cpp
    filtermanager.cpp
    filtersettings.cpp
)

ki18n_wrap_ui(choqok_filter addeditfilter_base.ui filterprefs.ui)

kcoreaddons_desktop_to_json(choqok_filter choqok_filter.desktop)

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

install(TARGETS choqok_filter DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES choqok_filter.desktop DESTINATION ${KDE_INSTALL_KSERVICESDIR})
install(FILES filterui.rc DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/choqok_filter)
