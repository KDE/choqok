set(choqok_nowlistening_PART_SRCS
    mpris.cpp
    nowlistening.cpp
)

kconfig_add_kcfg_files(choqok_nowlistening_PART_SRCS nowlisteningsettings.kcfgc)

add_library(choqok_nowlistening MODULE ${choqok_nowlistening_PART_SRCS})

kcoreaddons_desktop_to_json(choqok_nowlistening choqok_nowlistening.desktop)

target_link_libraries(choqok_nowlistening
PUBLIC
    Qt5::Core
    Qt5::DBus
    Qt5::Widgets
    KF5::CoreAddons
    KF5::I18n
    KF5::WidgetsAddons
    KF5::XmlGui
    choqok
)

install(TARGETS choqok_nowlistening DESTINATION ${PLUGIN_INSTALL_DIR})

set(kcm_choqok_nowlistening_PART_SRCS
    nowlisteningconfig.cpp
)

ki18n_wrap_ui(kcm_choqok_nowlistening_PART_SRCS nowlisteningprefs.ui)

kconfig_add_kcfg_files(kcm_choqok_nowlistening_PART_SRCS nowlisteningsettings.kcfgc)

add_library(kcm_choqok_nowlistening MODULE ${kcm_choqok_nowlistening_PART_SRCS})

kcoreaddons_desktop_to_json(kcm_choqok_nowlistening choqok_nowlistening_config.desktop)

target_link_libraries(kcm_choqok_nowlistening
PUBLIC
    Qt5::Widgets
    KF5::ConfigWidgets
    KF5::CoreAddons
    KF5::I18n
)

install(TARGETS kcm_choqok_nowlistening DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES nowlisteningsettings.kcfg DESTINATION ${KCFG_INSTALL_DIR})
install(FILES choqok_nowlistening_config.desktop choqok_nowlistening.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES nowlisteningui.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/choqok_nowlistening)
