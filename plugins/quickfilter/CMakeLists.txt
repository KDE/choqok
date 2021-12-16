add_library(choqok_quickfilter MODULE
    quickfilter.cpp
)

kcoreaddons_desktop_to_json(choqok_quickfilter choqok_quickfilter.desktop)

target_link_libraries(choqok_quickfilter
PUBLIC
    Qt5::Core
    Qt5::Widgets
    KF5::CoreAddons
    KF5::I18n
    KF5::XmlGui
    choqok
)

install(TARGETS choqok_quickfilter DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES choqok_quickfilter.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install(FILES quickfilterui.rc DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/choqok_quickfilter)
