kcoreaddons_add_plugin(choqok_quickfilter SOURCES quickfilter.cpp INSTALL_NAMESPACE "choqok_plugins")

target_link_libraries(choqok_quickfilter
PUBLIC
    Qt::Core
    Qt::Widgets
    KF5::CoreAddons
    KF5::I18n
    KF5::XmlGui
    choqok
)

install(FILES quickfilterui.rc DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/choqok_quickfilter)
