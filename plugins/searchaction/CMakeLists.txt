include_directories(
    ${CHOQOK_INCLUDES}
)

set(choqok_searchaction_PART_SRCS
    searchaction.cpp
)

add_library(choqok_searchaction MODULE ${choqok_searchaction_PART_SRCS})

kcoreaddons_desktop_to_json(choqok_searchaction choqok_searchaction.desktop)

target_link_libraries(choqok_searchaction
PUBLIC
    Qt5::Core
    Qt5::Widgets
    KF5::CoreAddons
    KF5::I18n
    KF5::WidgetsAddons
    KF5::XmlGui
    choqok
    twitterapihelper
)

install(TARGETS choqok_searchaction DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES choqok_searchaction.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES searchactionui.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/choqok_searchaction)
