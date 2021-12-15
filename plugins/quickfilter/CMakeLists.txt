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

install(TARGETS choqok_quickfilter DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES choqok_quickfilter.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES quickfilterui.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/choqok_quickfilter)
