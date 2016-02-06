
include_directories(
    ${CHOQOK_INCLUDES}
)

set(gnusocialapihelper_SRCS
    gnusocialapiaccount.cpp
    gnusocialapicomposerwidget.cpp
    gnusocialapiconversationtimelinewidget.cpp
    gnusocialapidebug.cpp
    gnusocialapidmessagedialog.cpp
    gnusocialapimicroblog.cpp
    gnusocialapipostwidget.cpp
    gnusocialapisearch.cpp
)

add_library(gnusocialapihelper SHARED ${gnusocialapihelper_SRCS})

target_link_libraries(gnusocialapihelper
PUBLIC
    Qt5::Core
    Qt5::Xml
    Qt5::Widgets
    KF5::CoreAddons
    KF5::I18n
    KF5::KIOCore
    KF5::KIOWidgets
    KF5::WidgetsAddons
    choqok
    twitterapihelper
    ${QTOAUTH_LIBRARY}
)

set_target_properties(gnusocialapihelper PROPERTIES
    VERSION 1.3.0
    SOVERSION 1
)

install(TARGETS gnusocialapihelper ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES
    gnusocialapiaccount.h
    gnusocialapicomposerwidget.h
    gnusocialapiconversationtimelinewidget.h
    gnusocialapidmessagedialog.h
    gnusocialapimicroblog.h
    gnusocialapipostwidget.h
    gnusocialapisearch.h
DESTINATION ${INCLUDE_INSTALL_DIR}/choqok/gnusocialapihelper COMPONENT Devel)
