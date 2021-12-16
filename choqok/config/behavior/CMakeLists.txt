#if(QtIndicate_FOUND)
#	add_definitions(-DQTINDICATE_BUILD)
#endif(QtIndicate_FOUND)

add_library(kcm_choqok_behaviorconfig MODULE
   behaviorconfig.cpp
   behaviorconfig_shorten.cpp
)

ecm_qt_declare_logging_category(kcm_choqok_behaviorconfig
    HEADER behaviordebug.h
    IDENTIFIER CHOQOK
    CATEGORY_NAME org.kde.choqok.config.behavior
    DESCRIPTION "choqok behavior config"
    EXPORT CHOQOK
)

ki18n_wrap_ui(kcm_choqok_behaviorconfig
   behaviorconfig_general.ui
   behaviorconfig_notifications.ui
   behaviorconfig_shorten_base.ui
)

kcoreaddons_desktop_to_json(kcm_choqok_behaviorconfig choqok_behaviorconfig.desktop)

target_link_libraries(kcm_choqok_behaviorconfig
PUBLIC
    Qt5::Widgets
    KF5::CoreAddons
    KF5::ConfigWidgets
    KF5::I18n
    KF5::KCMUtils
    KF5::XmlGui
    choqok
)

install(TARGETS kcm_choqok_behaviorconfig DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES choqok_behaviorconfig.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
