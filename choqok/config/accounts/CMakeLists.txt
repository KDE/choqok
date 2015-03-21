set(kcm_choqok_accountsconfig_PART_SRCS
    accountsdebug.cpp
    accountswidget.cpp
    addaccountdialog.cpp
    editaccountdialog.cpp
)

ki18n_wrap_ui(kcm_choqok_accountsconfig_PART_SRCS accountswidget_base.ui)

add_library(kcm_choqok_accountsconfig MODULE ${kcm_choqok_accountsconfig_PART_SRCS})

kcoreaddons_desktop_to_json(kcm_choqok_accountsconfig choqok_accountsconfig.desktop)

target_link_libraries(kcm_choqok_accountsconfig
PUBLIC
    Qt5::Core
    Qt5::Widgets
    KF5::CoreAddons
    KF5::ConfigWidgets
    KF5::I18n
    KF5::Service
    KF5::WidgetsAddons
    KF5::KDELibs4Support
    choqok
)

install(TARGETS kcm_choqok_accountsconfig DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES choqok_accountsconfig.desktop DESTINATION ${SERVICES_INSTALL_DIR})
