add_library(choqok_searchaction MODULE
    searchaction.cpp
)

kcoreaddons_desktop_to_json(choqok_searchaction choqok_searchaction.desktop)

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

install(TARGETS choqok_searchaction DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES choqok_searchaction.desktop DESTINATION ${KDE_INSTALL_KSERVICESDIR})
install(FILES searchactionui.rc DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/choqok_searchaction)
