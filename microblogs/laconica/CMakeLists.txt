
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
)

kde4_add_ui_files(choqok_laconica_SRCS laconicaeditaccount_base.ui )

kde4_add_plugin(choqok_laconica ${choqok_laconica_SRCS})

target_link_libraries(choqok_laconica ${KDE4_KIO_LIBS} choqok twitterapihelper)

# Install

install(TARGETS choqok_laconica DESTINATION ${PLUGIN_INSTALL_DIR})
install( FILES choqok_laconica.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

add_subdirectory(icons)
