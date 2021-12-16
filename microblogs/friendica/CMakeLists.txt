add_library(choqok_friendica MODULE
    friendicaeditaccount.cpp
    friendicamicroblog.cpp
)

ecm_qt_declare_logging_category(choqok_friendica
    HEADER friendicadebug.h
    IDENTIFIER CHOQOK
    CATEGORY_NAME org.kde.choqok.friendica
    DESCRIPTION "choqok friendica support"
    EXPORT CHOQOK
)

ki18n_wrap_ui(choqok_friendica friendicaeditaccount_base.ui)

kcoreaddons_desktop_to_json(choqok_friendica choqok_friendica.desktop)

target_link_libraries(choqok_friendica
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

install(TARGETS choqok_friendica DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES choqok_friendica.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

add_subdirectory(icons)
