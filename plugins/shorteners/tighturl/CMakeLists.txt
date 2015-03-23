set(choqok_tighturl_SRCS
    tighturl.cpp
)

add_library(choqok_tighturl MODULE ${choqok_tighturl_SRCS})

kcoreaddons_desktop_to_json(choqok_tighturl choqok_tighturl.desktop)

target_link_libraries(choqok_tighturl
PUBLIC
    Qt5::Core
    KF5::CoreAddons
    KF5::I18n
    KF5::KIOCore
    KF5::KDELibs4Support
    choqok
)

install(TARGETS choqok_tighturl DESTINATION ${PLUGIN_INSTALL_DIR})
install( FILES choqok_tighturl.desktop DESTINATION ${SERVICES_INSTALL_DIR})
