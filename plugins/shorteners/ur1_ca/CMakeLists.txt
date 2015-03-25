set(choqok_ur1_ca_SRCS
    ur1_ca.cpp
)

add_library(choqok_ur1_ca MODULE ${choqok_ur1_ca_SRCS})

kcoreaddons_desktop_to_json(choqok_ur1_ca choqok_ur1_ca.desktop)

target_link_libraries(choqok_ur1_ca
PUBLIC
    Qt5::Core
    KF5::CoreAddons
    KF5::I18n
    KF5::KIOCore
    choqok
)

install(TARGETS choqok_ur1_ca DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES choqok_ur1_ca.desktop DESTINATION ${SERVICES_INSTALL_DIR})
