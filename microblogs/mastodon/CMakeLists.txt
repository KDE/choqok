add_library(choqok_mastodon MODULE
    mastodonaccount.cpp
    mastodoncomposerwidget.cpp
    mastodondmessagedialog.cpp
    mastodoneditaccountwidget.cpp
    mastodonmicroblog.cpp
    mastodonoauth.cpp
    mastodonoauthreplyhandler.cpp
    mastodonpost.cpp
    mastodonpostwidget.cpp
)

ecm_qt_declare_logging_category(choqok_mastodon
    HEADER mastodondebug.h
    IDENTIFIER CHOQOK
    CATEGORY_NAME org.kde.choqok.mastodon
    DESCRIPTION "choqok mastodon support"
    EXPORT CHOQOK
)

ki18n_wrap_ui(choqok_mastodon
    mastodoneditaccountwidget.ui
)

kcoreaddons_desktop_to_json(choqok_mastodon choqok_mastodon.desktop)

target_link_libraries(choqok_mastodon
PUBLIC
    Qt5::Core
    Qt5::Gui
    Qt5::NetworkAuth
    Qt5::Widgets
    KF5::I18n
    KF5::KIOCore
    KF5::KIOWidgets
    KF5::WidgetsAddons
    qca-qt5
    choqok
)

install(TARGETS choqok_mastodon DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES choqok_mastodon.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})

add_subdirectory(icons)
