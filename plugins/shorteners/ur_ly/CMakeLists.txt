set( choqok_ur_ly_SRCS
ur_ly.cpp )

add_library(choqok_ur_ly MODULE ${choqok_ur_ly_SRCS})

kcoreaddons_desktop_to_json(choqok_ur_ly choqok_ur_ly.desktop)

target_link_libraries(choqok_ur_ly
PUBLIC
    Qt5::Core
    KF5::CoreAddons
    KF5::I18n
    KF5::KIOCore
    KF5::KDELibs4Support
    choqok
)

install(TARGETS choqok_ur_ly DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES choqok_ur_ly.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
