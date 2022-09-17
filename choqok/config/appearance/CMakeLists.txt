kcoreaddons_add_plugin(kcm_choqok_appearanceconfig SOURCES appearanceconfig.cpp INSTALL_NAMESPACE "choqok_kcms")

ecm_qt_declare_logging_category(kcm_choqok_appearanceconfig
    HEADER appearancedebug.h
    IDENTIFIER CHOQOK
    CATEGORY_NAME org.kde.choqok.config.appearance
    DESCRIPTION "choqok appearance config"
    EXPORT CHOQOK
)

ki18n_wrap_ui(kcm_choqok_appearanceconfig
    appearanceconfig_base.ui)

target_link_libraries(kcm_choqok_appearanceconfig
PUBLIC
    Qt::Widgets
    KF5::CoreAddons
    KF5::ConfigWidgets
    KF5::I18n
    choqok
)
