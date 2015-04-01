set(kcm_choqok_pluginconfig_PART_SRCS
    choqokpluginconfig.cpp
    pluginsdebug.cpp
)

add_library(kcm_choqok_pluginconfig MODULE ${kcm_choqok_pluginconfig_PART_SRCS})

kcoreaddons_desktop_to_json(kcm_choqok_pluginconfig choqok_pluginconfig.desktop)

target_link_libraries(kcm_choqok_pluginconfig
PUBLIC
    Qt5::Widgets
    KF5::ConfigWidgets
    KF5::CoreAddons
    KF5::I18n
    KF5::KCMUtils
    choqok
)

install(TARGETS kcm_choqok_pluginconfig DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES choqok_pluginconfig.desktop DESTINATION ${SERVICES_INSTALL_DIR})
