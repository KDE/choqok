add_library(choqok_pumpio MODULE
    pumpioaccount.cpp
    pumpiocomposerwidget.cpp
    pumpiodebug.cpp
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

ki18n_wrap_ui(choqok_pumpio
    pumpioeditaccountwidget.ui
    pumpiomessagedialog.ui
    pumpioshowthread.ui
)

kcoreaddons_desktop_to_json(choqok_pumpio choqok_pumpio.desktop)

target_link_libraries(choqok_pumpio
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

install(TARGETS choqok_pumpio DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES choqok_pumpio.desktop DESTINATION ${SERVICES_INSTALL_DIR})

add_subdirectory(icons)
