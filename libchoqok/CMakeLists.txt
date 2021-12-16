add_library(choqok SHARED)

set_target_properties(choqok PROPERTIES
    VERSION 1.3.0
    SOVERSION 1
)

if(MSVC)
    set_target_properties(choqok PROPERTIES OUTPUT_NAME libchoqok)
endif(MSVC)

generate_export_header(choqok)

ecm_qt_declare_logging_category(choqok
    HEADER libchoqokdebug.h
    IDENTIFIER CHOQOK
    CATEGORY_NAME org.kde.choqok.lib
    DESCRIPTION "choqok library"
    EXPORT CHOQOK
)

set(choqok_dbus_SRCS)
qt5_add_dbus_adaptor(choqok_dbus_SRCS org.kde.choqok.xml dbushandler.h Choqok::DbusHandler ChoqokAdaptor)

target_sources(choqok PRIVATE
    ${choqok_dbus_SRCS}
    application.cpp
    plugin.cpp
    shortener.cpp
    uploader.cpp
    account.cpp
    microblog.cpp
    pluginmanager.cpp
    shortenmanager.cpp
    accountmanager.cpp
    passwordmanager.cpp
    mediamanager.cpp
    notifymanager.cpp
    choqokuiglobal.cpp
    choqoktools.cpp
    dbushandler.cpp
    urlutils.cpp
    ui/microblogwidget.cpp
    ui/editaccountwidget.cpp
    ui/timelinewidget.cpp
    ui/postwidget.cpp
    ui/choqoktextedit.cpp
    ui/composerwidget.cpp
    ui/quickpost.cpp
    ui/choqokmainwindow.cpp
    ui/uploadmediadialog.cpp
    ui/textbrowser.cpp
    ui/choqoktabbar.cpp
)

#if(QtIndicate_FOUND)
#    set(choqok_LIB_SRCS ${choqok_LIB_SRCS} indicatormanager.cpp)
#    include_directories(${QTINDICATE_INCLUDE_DIRS})
#    add_definitions(-DQTINDICATE_BUILD)
#endif(QtIndicate_FOUND)

kconfig_add_kcfg_files(choqok
    choqokbehaviorsettings.kcfgc
    choqokappearancesettings.kcfgc
)

ki18n_wrap_ui(choqok ui/uploadmedia_base.ui)

target_link_libraries(choqok
PUBLIC
    Qt5::Core
    Qt5::DBus
    Qt5::NetworkAuth
    Qt5::Widgets
    KF5::ConfigCore
    KF5::Emoticons
    KF5::GuiAddons
    KF5::I18n
    KF5::KIOCore
    KF5::KIOFileWidgets
    KF5::KCMUtils
    KF5::JobWidgets
    KF5::Notifications
    KF5::SonnetCore
    KF5::TextWidgets
    KF5::XmlGui
    KF5::Wallet
)

#if(QtIndicate_FOUND)
#       add_definitions(-DXDG_APPS_INSTALL_DIR=${KDE_INSTALL_APPDIR})
#       target_link_libraries(choqok ${QTINDICATE_LIBRARY})
#endif(QtIndicate_FOUND)

target_include_directories(choqok PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ui>")

install(FILES choqokbehaviorsettings.kcfg choqokappearancesettings.kcfg
        DESTINATION ${KDE_INSTALL_KCFGDIR})

install(TARGETS choqok ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES choqokplugin.desktop choqokshortenerplugin.desktop
              choqokuploaderplugin.desktop DESTINATION ${KDE_INSTALL_KSERVICETYPES5DIR})

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/choqokbehaviorsettings.h
    ${CMAKE_CURRENT_BINARY_DIR}/choqokappearancesettings.h
    ${CMAKE_CURRENT_BINARY_DIR}/choqok_export.h
    application.h
    account.h
    accountmanager.h
    choqoktypes.h
    choqokuiglobal.h
    mediamanager.h
    microblog.h
    notifymanager.h
    passwordmanager.h
    plugin.h
    pluginmanager.h
    shortener.h
    uploader.h
    shortenmanager.h
    dbushandler.h
    choqoktools.h
    ui/choqoktextedit.h
    ui/composerwidget.h
    ui/editaccountwidget.h
    ui/choqokmainwindow.h
    ui/microblogwidget.h
    ui/postwidget.h
    ui/quickpost.h
    ui/timelinewidget.h
    ui/uploadmediadialog.h
    ui/textbrowser.h
    ui/choqoktabbar.h
DESTINATION ${KDE_INSTALL_INCLUDEDIR}/choqok COMPONENT Devel)
