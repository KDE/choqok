add_library(choqok_laconica MODULE
    laconicaeditaccount.cpp
    laconicamicroblog.cpp
)

ecm_qt_declare_logging_category(choqok_laconica
    HEADER laconicadebug.h
    IDENTIFIER CHOQOK
    CATEGORY_NAME org.kde.choqok.laconica
    DESCRIPTION "choqok laconica support"
    EXPORT CHOQOK
)

ki18n_wrap_ui(choqok_laconica laconicaeditaccount_base.ui)

kcoreaddons_desktop_to_json(choqok_laconica choqok_laconica.desktop)

target_link_libraries(choqok_laconica
PUBLIC
    Qt5::Core
    Qt5::NetworkAuth
    Qt5::Widgets
    KF5::CoreAddons
    KF5::I18n
    KF5::KIOCore
    KF5::KIOWidgets
    choqok
    gnusocialapihelper
)

install(TARGETS choqok_laconica DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES choqok_laconica.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR})

add_subdirectory(icons)
