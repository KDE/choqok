
include_directories(
${CHOQOK_INCLUDES}
${CMAKE_CURRENT_BINARY_DIR}
)

set( choqok_twitter_SRCS 
twitteraccount.cpp
twittermicroblog.cpp
twitterpostwidget.cpp
twittersearch.cpp
twittereditaccount.cpp
twitterlistdialog.cpp
twittertimelinewidget.cpp
)

kde4_add_ui_files(choqok_twitter_SRCS
twittereditaccount_base.ui
twitterlistdialog_base.ui
)

kde4_add_plugin(choqok_twitter ${choqok_twitter_SRCS})

target_link_libraries(choqok_twitter ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${QJSON_LIBRARY} ${QTOAUTH_LIBRARY} choqok twitterapihelper)

# Install

install(TARGETS choqok_twitter DESTINATION ${PLUGIN_INSTALL_DIR})
install( FILES choqok_twitter.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

add_subdirectory(icons)