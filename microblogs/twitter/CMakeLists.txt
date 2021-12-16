add_library(choqok_twitter MODULE
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

kcoreaddons_desktop_to_json(choqok_twitter choqok_twitter.desktop)

target_link_libraries(choqok_twitter
    Qt5::Core
    Qt5::NetworkAuth
    Qt5::Widgets
    KF5::CoreAddons
    KF5::I18n
    KF5::KIOCore
    KF5::KIOWidgets
    KF5::WidgetsAddons
    qca-qt5
    choqok
    twitterapihelper
)

install(TARGETS choqok_twitter DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES choqok_twitter.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})

add_subdirectory(icons)
