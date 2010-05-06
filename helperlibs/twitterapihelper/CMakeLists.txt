
include_directories(
${CHOQOK_INCLUDES}
${QJSON_INCLUDE_DIR}
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
)

kde4_add_library(twitterapihelper SHARED ${twitterhelper_LIB_SRCS})

target_link_libraries(twitterapihelper ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KUTILS_LIBS} ${QJSON_LIBRARY} choqok)

set_target_properties(twitterapihelper PROPERTIES
VERSION 0.7
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
DESTINATION ${INCLUDE_INSTALL_DIR}/choqok/twitterapihelper COMPONENT Devel)
