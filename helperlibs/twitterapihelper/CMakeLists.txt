
include_directories(
${CHOQOK_INCLUDES}
)

set(twitterhelper_LIB_SRCS
twitterapiaccount.cpp
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
)

# kde4_add_kcfg_files(twitterhelper_LIB_SRCS
# twitterapisettings.kcfgc
# )

kde4_add_library(twitterapihelper SHARED ${twitterhelper_LIB_SRCS})

target_link_libraries( twitterapihelper
                       ${QT_QTNETWORK_LIBRARY}
                       ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KUTILS_LIBS}
                       ${QTOAUTH_LIBRARY} ${QJSON_LIBRARY}
                       choqok)

set_target_properties(twitterapihelper PROPERTIES
VERSION 0.9.5
SOVERSION 0
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
DESTINATION ${INCLUDE_INSTALL_DIR}/choqok/twitterapihelper COMPONENT Devel)
