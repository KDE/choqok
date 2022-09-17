kcoreaddons_add_plugin(choqok_ocs SOURCES
    ocsaccount.cpp
    ocsmicroblog.cpp
    ocsconfigurewidget.cpp
    INSTALL_NAMESPACE "choqok_plugins"
)

ecm_qt_declare_logging_category(choqok_ocs
    HEADER ocsdebug.h
    IDENTIFIER CHOQOK
    CATEGORY_NAME org.kde.choqok.ocs
    DESCRIPTION "choqok ocs support"
    EXPORT CHOQOK
)

ki18n_wrap_ui(choqok_ocs
    ocsconfigurebase.ui
)

target_link_libraries(choqok_ocs
PUBLIC
    KF5::Attica
    KF5::ConfigCore
    KF5::CoreAddons
    KF5::I18n
    KF5::WidgetsAddons
    choqok
)

add_subdirectory(icons)
