include_directories(
    ${CHOQOK_INCLUDES}
)

set(choqok_filter_PART_SRCS
    addeditfilter.cpp
    configurefilters.cpp
    filter.cpp
    filtermanager.cpp
    filtersettings.cpp
)

ki18n_wrap_ui(choqok_filter_PART_SRCS addeditfilter_base.ui filterprefs.ui)

add_library(choqok_filter MODULE ${choqok_filter_PART_SRCS})

kcoreaddons_desktop_to_json(choqok_filter choqok_filter.desktop)

target_link_libraries(choqok_filter
PUBLIC
    Qt5::Core
    Qt5::Widgets
    KF5::ConfigCore
    KF5::CoreAddons
    KF5::I18n
    KF5::KIOCore
    KF5::WidgetsAddons
    KF5::XmlGui
    choqok
    twitterapihelper
)

install(TARGETS choqok_filter DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES choqok_filter.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES filterui.rc  DESTINATION  ${DATA_INSTALL_DIR}/choqok_filter)
