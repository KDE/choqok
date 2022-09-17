kcoreaddons_add_plugin(choqok_friendica SOURCES
    friendicaeditaccount.cpp
    friendicamicroblog.cpp
    INSTALL_NAMESPACE "choqok_plugins"
)

ecm_qt_declare_logging_category(choqok_friendica
    HEADER friendicadebug.h
    IDENTIFIER CHOQOK
    CATEGORY_NAME org.kde.choqok.friendica
    DESCRIPTION "choqok friendica support"
    EXPORT CHOQOK
)

ki18n_wrap_ui(choqok_friendica friendicaeditaccount_base.ui)

target_link_libraries(choqok_friendica
PUBLIC
    Qt::Core
    Qt::NetworkAuth
    Qt::Widgets
    KF5::CoreAddons
    KF5::I18n
    KF5::KIOCore
    KF5::KIOWidgets
    choqok
    gnusocialapihelper
)

add_subdirectory(icons)
