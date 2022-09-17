kcoreaddons_add_plugin(kcm_choqok_pluginconfig SOURCES choqokpluginconfig.cpp INSTALL_NAMESPACE "choqok_kcms")

ecm_qt_declare_logging_category(kcm_choqok_pluginconfig
    HEADER pluginsdebug.h
    IDENTIFIER CHOQOK
    CATEGORY_NAME org.kde.choqok.config.plugins
    DESCRIPTION "choqok plugins config"
    EXPORT CHOQOK
)

target_link_libraries(kcm_choqok_pluginconfig
PUBLIC
    Qt::Widgets
    KF5::ConfigWidgets
    KF5::CoreAddons
    KF5::I18n
    KF5::KCMUtils
    choqok
)
