add_library(twitterapihelper SHARED)
set_target_properties(twitterapihelper PROPERTIES
    VERSION 1.3.0
    SOVERSION 1
)

generate_export_header(twitterapihelper)

ecm_qt_declare_logging_category(twitterapihelper
    HEADER twitterapidebug.h
    IDENTIFIER CHOQOK
    CATEGORY_NAME org.kde.choqok.twitterapi
    DESCRIPTION "choqok twitterapi library"
    EXPORT CHOQOK
)

target_sources(twitterapihelper PRIVATE
    twitterapiaccount.cpp
    twitterapicomposerwidget.cpp
    twitterapipostwidget.cpp
    twitterapidmessagedialog.cpp
    twitterapimicroblog.cpp
    twitterapimicroblogwidget.cpp
    twitterapioauth.cpp
    twitterapioauthreplyhandler.cpp
    twitterapisearch.cpp
    twitterapisearchdialog.cpp
    twitterapisearchtimelinewidget.cpp
    twitterapishowthread.cpp
    twitterapitextedit.cpp
    twitterapitimelinewidget.cpp
    twitterapiwhoiswidget.cpp
)

# kconfig_add_kcfg_files(twitterapihelper
# twitterapisettings.kcfgc
# )

target_link_libraries( twitterapihelper
PUBLIC
    Qt5::Core
    Qt5::Gui
    Qt5::NetworkAuth
    Qt5::Widgets
    KF5::ConfigCore
    KF5::KIOCore
    KF5::KIOWidgets
    KF5::I18n
    KF5::WidgetsAddons
    qca-qt5
    choqok
)

install(TARGETS twitterapihelper ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES
    twitterapiaccount.h
    twitterapicomposerwidget.h
    twitterapidmessagedialog.h
    twitterapimicroblog.h
    twitterapimicroblogwidget.h
    twitterapioauth.h
    twitterapioauthreplyhandler.h
    twitterapipostwidget.h
    twitterapisearch.h
    twitterapisearchdialog.h
    twitterapisearchtimelinewidget.h
    twitterapishowthread.h
    twitterapitextedit.h
    twitterapitimelinewidget.h
    twitterapiwhoiswidget.h
    ${CMAKE_CURRENT_BINARY_DIR}/twitterapihelper_export.h
DESTINATION ${INCLUDE_INSTALL_DIR}/choqok/twitterapihelper COMPONENT Devel)
