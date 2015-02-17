
include_directories(
${CHOQOK_INCLUDES}
${CMAKE_CURRENT_BINARY_DIR}
)

set( choqok_twitter_SRCS 
twittercomposerwidget.cpp
twitteraccount.cpp
twittermicroblog.cpp
twitterpostwidget.cpp
twittersearch.cpp
twittereditaccount.cpp
twitterlistdialog.cpp
twittertimelinewidget.cpp
)

ki18n_wrap_ui(choqok_twitter_SRCS
twittereditaccount_base.ui
twitterlistdialog_base.ui
)

add_library(choqok_twitter MODULE ${choqok_twitter_SRCS})

target_link_libraries(choqok_twitter  KF5::KIOCore ${QJSON_LIBRARY} ${QTOAUTH_LIBRARY} choqok twitterapihelper)

# Install

install(TARGETS choqok_twitter DESTINATION ${PLUGIN_INSTALL_DIR})
install( FILES choqok_twitter.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

add_subdirectory(icons)
