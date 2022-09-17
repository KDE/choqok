kcoreaddons_add_plugin(choqok_pumpio SOURCES
    pumpioaccount.cpp
    pumpiocomposerwidget.cpp
    pumpioeditaccountwidget.cpp
    pumpiomessagedialog.cpp
    pumpiomicroblog.cpp
    pumpiomicroblogwidget.cpp
    pumpiooauth.cpp
    pumpiooauthreplyhandler.cpp
    pumpiopost.cpp
    pumpiopostwidget.cpp
    pumpioshowthread.cpp
    INSTALL_NAMESPACE "choqok_plugins"
)

ecm_qt_declare_logging_category(choqok_pumpio
    HEADER pumpiodebug.h
    IDENTIFIER CHOQOK
    CATEGORY_NAME org.kde.choqok.pumpio
    DESCRIPTION "choqok pumpio support"
    EXPORT CHOQOK
)

ki18n_wrap_ui(choqok_pumpio
    pumpioeditaccountwidget.ui
    pumpiomessagedialog.ui
    pumpioshowthread.ui
)

target_link_libraries(choqok_pumpio
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
