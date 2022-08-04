add_library(choqok_pumpio MODULE
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

kcoreaddons_desktop_to_json(choqok_pumpio choqok_pumpio.desktop)

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

install(TARGETS choqok_pumpio DESTINATION ${KDE_INSTALL_PLUGINDIR})
install(FILES choqok_pumpio.desktop DESTINATION ${KDE_INSTALL_KSERVICESDIR})

add_subdirectory(icons)
