include_directories(
${CHOQOK_INCLUDES}
)

set( choqok_ur1_ca_SRCS
ur1_ca.cpp )

add_library(choqok_ur1_ca MODULE ${choqok_ur1_ca_SRCS})

target_link_libraries(choqok_ur1_ca KF5::KIOCore choqok)

install(TARGETS choqok_ur1_ca DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES choqok_ur1_ca.desktop  DESTINATION ${SERVICES_INSTALL_DIR})