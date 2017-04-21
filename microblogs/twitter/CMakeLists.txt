include_directories(
    ${CHOQOK_INCLUDES}
)

set(choqok_twitter_SRCS
    twittercomposerwidget.cpp
    twitteraccount.cpp
    twitterdebug.cpp
    twitterdmessagedialog.cpp
    twittermicroblog.cpp
    twitterpostwidget.cpp
    twittersearch.cpp
    twittertextedit.cpp
    twittereditaccount.cpp
    twitterlistdialog.cpp
    twittertimelinewidget.cpp
)

ki18n_wrap_ui(choqok_twitter_SRCS
    twittereditaccount_base.ui
    twitterlistdialog_base.ui
)

add_library(choqok_twitter MODULE ${choqok_twitter_SRCS})

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

install(TARGETS choqok_twitter DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES choqok_twitter.desktop DESTINATION ${SERVICES_INSTALL_DIR})

add_subdirectory(icons)
