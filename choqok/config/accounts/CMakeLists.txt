add_library(kcm_choqok_accountsconfig MODULE
    accountswidget.cpp
    addaccountdialog.cpp
    editaccountdialog.cpp
)

ecm_qt_declare_logging_category(kcm_choqok_accountsconfig
    HEADER accountsdebug.h
    IDENTIFIER CHOQOK
    CATEGORY_NAME org.kde.choqok.config.accounts
    DESCRIPTION "choqok accounts config"
    EXPORT CHOQOK
)

ki18n_wrap_ui(kcm_choqok_accountsconfig accountswidget_base.ui)

kcoreaddons_desktop_to_json(kcm_choqok_accountsconfig choqok_accountsconfig.desktop)

target_link_libraries(kcm_choqok_accountsconfig
PUBLIC
    Qt::Core
    Qt::Widgets
    KF5::CoreAddons
    KF5::ConfigWidgets
    KF5::I18n
    KF5::Service
    KF5::WidgetsAddons
    choqok
)

install(TARGETS kcm_choqok_accountsconfig DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES choqok_accountsconfig.desktop DESTINATION ${KDE_INSTALL_KSERVICESDIR})
