kcoreaddons_add_plugin(kcm_choqok_behaviorconfig SOURCES behaviorconfig.cpp behaviorconfig_shorten.cpp INSTALL_NAMESPACE "choqok_kcms")

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

target_link_libraries(kcm_choqok_behaviorconfig
PUBLIC
    Qt::Widgets
    KF5::CoreAddons
    KF5::ConfigWidgets
    KF5::I18n
    KF5::KCMUtils
    KF5::XmlGui
    choqok
)
