set(kcm_choqok_appearanceconfig_PART_SRCS
    appearanceconfig.cpp
    appearancedebug.cpp
)

ki18n_wrap_ui(kcm_choqok_appearanceconfig_PART_SRCS
    appearanceconfig_base.ui)

add_library(kcm_choqok_appearanceconfig MODULE ${kcm_choqok_appearanceconfig_PART_SRCS})

kcoreaddons_desktop_to_json(kcm_choqok_appearanceconfig choqok_appearanceconfig.desktop)

target_link_libraries(kcm_choqok_appearanceconfig
PUBLIC
    Qt5::Widgets
    KF5::CoreAddons
    KF5::ConfigWidgets
    KF5::I18n
    choqok
)

install(TARGETS kcm_choqok_appearanceconfig DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES choqok_appearanceconfig.desktop DESTINATION ${SERVICES_INSTALL_DIR})
