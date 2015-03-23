set(choqok_goo_gl_SRCS
    goo_gl.cpp
)

add_library(choqok_goo_gl MODULE ${choqok_goo_gl_SRCS})

kcoreaddons_desktop_to_json(choqok_goo_gl choqok_goo_gl.desktop)

target_link_libraries(choqok_goo_gl
PUBLIC
    Qt5::Core
    KF5::CoreAddons
    KF5::I18n
    KF5::KIOCore
    KF5::KDELibs4Support
    choqok
)

install(TARGETS choqok_goo_gl DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES choqok_goo_gl.desktop DESTINATION ${SERVICES_INSTALL_DIR})
