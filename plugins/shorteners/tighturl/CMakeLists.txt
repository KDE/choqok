include_directories(
${CHOQOK_INCLUDES}
)

set( choqok_tighturl_SRCS
tighturl.cpp )

add_library(choqok_tighturl MODULE ${choqok_tighturl_SRCS})

target_link_libraries(choqok_tighturl KF5::KIOCore choqok)

install(TARGETS choqok_tighturl DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES choqok_tighturl.desktop  DESTINATION ${SERVICES_INSTALL_DIR})