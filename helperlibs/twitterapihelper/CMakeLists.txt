include_directories(
    ${CHOQOK_INCLUDES}
)

set(twitterhelper_LIB_SRCS
    twitterapiaccount.cpp
    twitterapicomposerwidget.cpp
    twitterapidebug.cpp
    twitterapipostwidget.cpp
    twitterapidmessagedialog.cpp
    twitterapimicroblog.cpp
    twitterapimicroblogwidget.cpp
    twitterapisearch.cpp
    twitterapisearchdialog.cpp
    twitterapisearchtimelinewidget.cpp
    twitterapishowthread.cpp
    twitterapitextedit.cpp
    twitterapitimelinewidget.cpp
    twitterapiwhoiswidget.cpp
)

# kconfig_add_kcfg_files(twitterhelper_LIB_SRCS
# twitterapisettings.kcfgc
# )

add_library(twitterapihelper SHARED ${twitterhelper_LIB_SRCS})

target_link_libraries( twitterapihelper
PUBLIC
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    KF5::ConfigCore
    KF5::KIOCore
    KF5::KIOWidgets
    KF5::I18n
    KF5::WidgetsAddons
    ${QTOAUTH_LIBRARY}
    qca-qt5
    choqok
)

set_target_properties(twitterapihelper PROPERTIES
VERSION 1.3.0
SOVERSION 1
)

install(TARGETS twitterapihelper ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES
    twitterapiaccount.h
    twitterapicomposerwidget.h
    twitterapidmessagedialog.h
    twitterapimicroblog.h
    twitterapimicroblogwidget.h
    twitterapipostwidget.h
    twitterapisearch.h
    twitterapisearchdialog.h
    twitterapisearchtimelinewidget.h
    twitterapishowthread.h
    twitterapitextedit.h
    twitterapitimelinewidget.h
    twitterapiwhoiswidget.h
DESTINATION ${INCLUDE_INSTALL_DIR}/choqok/twitterapihelper COMPONENT Devel)
