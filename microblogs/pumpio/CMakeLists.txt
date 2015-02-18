include_directories(
    ${CHOQOK_INCLUDES}
)

set(choqok_pumpio_SRCS
    pumpioaccount.cpp
    pumpiocomposerwidget.cpp
    pumpiodebug.cpp
    pumpioeditaccountwidget.cpp
    pumpiomessagedialog.cpp
    pumpiomicroblog.cpp
    pumpiomicroblogwidget.cpp
    pumpiopost.cpp
    pumpiopostwidget.cpp
    pumpioshowthread.cpp
)

ki18n_wrap_ui(choqok_pumpio_SRCS
    pumpioeditaccountwidget.ui
    pumpiomessagedialog.ui
    pumpioshowthread.ui
)

add_library(choqok_pumpio MODULE ${choqok_pumpio_SRCS})

kcoreaddons_desktop_to_json(choqok_pumpio choqok_pumpio.desktop)

target_link_libraries(choqok_pumpio
PUBLIC
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    KF5::I18n
    KF5::KIOCore
    KF5::WidgetsAddons
    KF5::KDELibs4Support
    qca-qt5
    ${QTOAUTH_LIBRARY}
    choqok
)

install(TARGETS choqok_pumpio DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES choqok_pumpio.desktop DESTINATION ${SERVICES_INSTALL_DIR})

add_subdirectory(icons)
