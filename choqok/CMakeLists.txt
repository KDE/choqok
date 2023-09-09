include(DBusMacros)

add_subdirectory(config)

configure_file(config-choqok.h.in ${CMAKE_CURRENT_BINARY_DIR}/config-choqok.h)

add_executable(choqok_bin)
set_target_properties(choqok_bin PROPERTIES OUTPUT_NAME choqok)

ecm_qt_declare_logging_category(choqok_bin
    HEADER choqokdebug.h
    IDENTIFIER CHOQOK
    CATEGORY_NAME org.kde.choqok
    DESCRIPTION "choqok"
    EXPORT CHOQOK
)

target_sources(choqok_bin PRIVATE
   choqokapplication.cpp
   mainwindow.cpp
   main.cpp
   systrayicon.cpp

   choqok.qrc
)

# kconfig_add_kcfg_files(choqok_bin settings.kcfgc)
dbus_add_activation_service(org.kde.choqok.service.in)

target_link_libraries(choqok_bin
PUBLIC
    Qt::Core
    Qt::Widgets
    KF5::ConfigWidgets
    KF5::CoreAddons
    KF5::I18n
    KF5::KCMUtils
    KF5::Notifications
    KF5::NotifyConfig
    KF5::WidgetsAddons
    KF5::XmlGui
    choqok
)
if(HAVE_KGLOBALACCEL)
    target_link_libraries(choqok_bin PUBLIC KF5::GlobalAccel)
endif()

install(TARGETS choqok_bin ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS org.kde.choqok.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES org.kde.choqok.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
# install(FILES choqok.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
install(FILES choqok.notifyrc DESTINATION ${KDE_INSTALL_KNOTIFYRCDIR})
