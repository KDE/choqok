kcoreaddons_add_plugin(choqok_laconica SOURCES
    laconicaeditaccount.cpp
    laconicamicroblog.cpp
    INSTALL_NAMESPACE "choqok_plugins"
)

ecm_qt_declare_logging_category(choqok_laconica
    HEADER laconicadebug.h
    IDENTIFIER CHOQOK
    CATEGORY_NAME org.kde.choqok.laconica
    DESCRIPTION "choqok laconica support"
    EXPORT CHOQOK
)

ki18n_wrap_ui(choqok_laconica laconicaeditaccount_base.ui)

target_link_libraries(choqok_laconica
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
