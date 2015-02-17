include(DBusMacros)

include_directories(
${CHOQOK_INCLUDES}
)

add_subdirectory( config )

set(choqok_SRCS
   choqokapplication.cpp
   mainwindow.cpp
   main.cpp
   systrayicon.cpp
)

# ki18n_wrap_ui(choqok_SRCS accounts/accountswidget_base.ui)

# kconfig_add_kcfg_files(choqok_SRCS settings.kcfgc )
dbus_add_activation_service(org.kde.choqok.service.in)

add_executable(choqok_bin ${choqok_SRCS})

set_target_properties(choqok_bin PROPERTIES OUTPUT_NAME choqok)

target_link_libraries(choqok_bin
PUBLIC
    Qt5::Core
    Qt5::Widgets
    KF5::ConfigWidgets
    KF5::CoreAddons
    KF5::GlobalAccel
    KF5::I18n
    KF5::NotifyConfig
    KF5::WidgetsAddons
    KF5::XmlGui
    KF5::KDELibs4Support
    choqok
)

install(TARGETS choqok_bin ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install( PROGRAMS choqok.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES choqok.appdata.xml DESTINATION  ${SHARE_INSTALL_PREFIX}/appdata )
# install( FILES choqok.kcfg  DESTINATION  ${KCFG_INSTALL_DIR} )
install( FILES choqokui.rc DESTINATION  ${KXMLGUI_INSTALL_DIR}/choqok )
install( FILES choqok.notifyrc  DESTINATION  ${KNOTIFYRC_INSTALL_DIR}/choqok )
