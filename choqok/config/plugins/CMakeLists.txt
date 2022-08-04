add_library(kcm_choqok_pluginconfig MODULE
    choqokpluginconfig.cpp
)

ecm_qt_declare_logging_category(kcm_choqok_pluginconfig
    HEADER pluginsdebug.h
    IDENTIFIER CHOQOK
    CATEGORY_NAME org.kde.choqok.config.plugins
    DESCRIPTION "choqok plugins config"
    EXPORT CHOQOK
)

kcoreaddons_desktop_to_json(kcm_choqok_pluginconfig choqok_pluginconfig.desktop)

target_link_libraries(kcm_choqok_pluginconfig
PUBLIC
    Qt::Widgets
    KF5::ConfigWidgets
    KF5::CoreAddons
    KF5::I18n
    KF5::KCMUtils
    choqok
)

install(TARGETS kcm_choqok_pluginconfig DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES choqok_pluginconfig.desktop DESTINATION ${KDE_INSTALL_KSERVICESDIR})
