include_directories(
    ${CHOQOK_INCLUDES}
)

set(choqok_mastodon_SRCS
    mastodonaccount.cpp
    mastodoncomposerwidget.cpp
    mastodondebug.cpp
    mastodondmessagedialog.cpp
    mastodoneditaccountwidget.cpp
    mastodonmicroblog.cpp
    mastodonoauth.cpp
    mastodonoauthreplyhandler.cpp
    mastodonpost.cpp
    mastodonpostwidget.cpp
)

ki18n_wrap_ui(choqok_mastodon_SRCS
    mastodoneditaccountwidget.ui
)

add_library(choqok_mastodon MODULE ${choqok_mastodon_SRCS})

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

install(TARGETS choqok_mastodon DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES choqok_mastodon.desktop DESTINATION ${SERVICES_INSTALL_DIR})

add_subdirectory(icons)
