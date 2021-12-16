add_library(choqok_ocs MODULE
    ocsaccount.cpp
    ocsmicroblog.cpp
    ocsconfigurewidget.cpp
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

kcoreaddons_desktop_to_json(choqok_ocs choqok_ocs.desktop)

target_link_libraries(choqok_ocs
PUBLIC
    KF5::Attica
    KF5::ConfigCore
    KF5::CoreAddons
    KF5::I18n
    KF5::WidgetsAddons
    choqok
)

install(TARGETS choqok_ocs DESTINATION ${KDE_INSTALL_PLUGINDIR})
install( FILES choqok_ocs.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR})

add_subdirectory(icons)
