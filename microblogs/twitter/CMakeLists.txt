kcoreaddons_add_plugin(choqok_twitter SOURCES
    twittercomposerwidget.cpp
    twitteraccount.cpp
    twitterdmessagedialog.cpp
    twittermicroblog.cpp
    twitterpostwidget.cpp
    twittersearch.cpp
    twittertextedit.cpp
    twittereditaccount.cpp
    twitterlistdialog.cpp
    twittertimelinewidget.cpp
    INSTALL_NAMESPACE "choqok_plugins"
)

ecm_qt_declare_logging_category(choqok_twitter
    HEADER twitterdebug.h
    IDENTIFIER CHOQOK
    CATEGORY_NAME org.kde.choqok.twitter
    DESCRIPTION "choqok twitter support"
    EXPORT CHOQOK
)

ki18n_wrap_ui(choqok_twitter
    twittereditaccount_base.ui
    twitterlistdialog_base.ui
)

target_link_libraries(choqok_twitter
    Qt::Core
    Qt::NetworkAuth
    Qt::Widgets
    KF5::CoreAddons
    KF5::I18n
    KF5::KIOCore
    KF5::KIOWidgets
    KF5::WidgetsAddons
    qca-qt${QT_MAJOR_VERSION}
    choqok
    twitterapihelper
)

add_subdirectory(icons)
