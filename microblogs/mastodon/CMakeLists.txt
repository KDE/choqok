kcoreaddons_add_plugin(choqok_mastodon SOURCES
    mastodonaccount.cpp
    mastodoncomposerwidget.cpp
    mastodondmessagedialog.cpp
    mastodoneditaccountwidget.cpp
    mastodonmicroblog.cpp
    mastodonoauth.cpp
    mastodonoauthreplyhandler.cpp
    mastodonpost.cpp
    mastodonpostwidget.cpp
    INSTALL_NAMESPACE "choqok_plugins"
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

target_link_libraries(choqok_mastodon
PUBLIC
    Qt::Core
    Qt::Gui
    Qt::NetworkAuth
    Qt::Widgets
    KF5::I18n
    KF5::KIOCore
    KF5::KIOWidgets
    KF5::WidgetsAddons
    qca-qt${QT_MAJOR_VERSION}
    choqok
)

add_subdirectory(icons)
