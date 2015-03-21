#if(QtIndicate_FOUND)
#	add_definitions(-DQTINDICATE_BUILD)
#endif(QtIndicate_FOUND)

set(kcm_choqok_behaviorconfig_PART_SRCS
   behaviorconfig.cpp
   behaviorconfig_shorten.cpp
   behaviordebug.cpp
)

ki18n_wrap_ui(kcm_choqok_behaviorconfig_PART_SRCS
   behaviorconfig_general.ui
   behaviorconfig_notifications.ui
   behaviorconfig_shorten_base.ui
)

add_library(kcm_choqok_behaviorconfig MODULE ${kcm_choqok_behaviorconfig_PART_SRCS})

kcoreaddons_desktop_to_json(kcm_choqok_behaviorconfig choqok_behaviorconfig.desktop)

target_link_libraries(kcm_choqok_behaviorconfig
PUBLIC
    Qt5::Widgets
    KF5::CoreAddons
    KF5::ConfigWidgets
    KF5::I18n
    KF5::KCMUtils
    KF5::XmlGui
    KF5::KDELibs4Support
    choqok
)

install(TARGETS kcm_choqok_behaviorconfig DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES choqok_behaviorconfig.desktop DESTINATION ${SERVICES_INSTALL_DIR})
