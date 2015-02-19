
include_directories(
${CHOQOK_INCLUDES}
)

set(twitterhelper_LIB_SRCS
twitterapiaccount.cpp
twitterapidebug.cpp
twitterapimicroblog.cpp
twitterapipostwidget.cpp
twitterapidmessagedialog.cpp
twitterapisearch.cpp
twitterapisearchdialog.cpp
twitterapimicroblogwidget.cpp
twitterapisearchtimelinewidget.cpp
twitterapiwhoiswidget.cpp
twitterapishowthread.cpp
twitterapitextedit.cpp
twitterapicomposerwidget.cpp
twitterapitimelinewidget.cpp
)

# kconfig_add_kcfg_files(twitterhelper_LIB_SRCS
# twitterapisettings.kcfgc
# )

add_library(twitterapihelper SHARED ${twitterhelper_LIB_SRCS})

target_link_libraries( twitterapihelper
PUBLIC
    Qt5::Network
    KF5::KIOCore
    ${QTOAUTH_LIBRARY}
    qca-qt5
    choqok
)

set_target_properties(twitterapihelper PROPERTIES
VERSION 1.3.0
SOVERSION 1
)

install(TARGETS twitterapihelper ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES
    twitterapiaccount.h
    twitterapidmessagedialog.h
    twitterapimicroblog.h
    twitterapimicroblogwidget.h
    twitterapipostwidget.h
    twitterapisearch.h
    twitterapisearchdialog.h
    twitterapisearchtimelinewidget.h
    twitterapiwhoiswidget.h
    twitterapishowthread.h
    twitterapitextedit.h
    twitterapicomposerwidget.h
    twitterapitimelinewidget.h
DESTINATION ${INCLUDE_INSTALL_DIR}/choqok/twitterapihelper COMPONENT Devel)
