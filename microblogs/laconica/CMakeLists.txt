
include_directories(
${CHOQOK_INCLUDES}
${CMAKE_CURRENT_BINARY_DIR}
)

set( choqok_laconica_SRCS
laconicaaccount.cpp
laconicamicroblog.cpp
laconicasearch.cpp
laconicaeditaccount.cpp
laconicapostwidget.cpp
laconicacomposerwidget.cpp
laconicaconversationtimelinewidget.cpp
)

ki18n_wrap_ui(choqok_laconica_SRCS laconicaeditaccount_base.ui )

add_library(choqok_laconica MODULE ${choqok_laconica_SRCS})

target_link_libraries(choqok_laconica KF5::KIOCore ${QJSON_LIBRARY} choqok twitterapihelper ${QTOAUTH_LIBRARY})

# Install

install(TARGETS choqok_laconica DESTINATION ${PLUGIN_INSTALL_DIR})
install( FILES choqok_laconica.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

add_subdirectory(icons)
