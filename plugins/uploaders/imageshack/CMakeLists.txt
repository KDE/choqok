set(choqok_imageshack_PART_SRCS
    imageshack.cpp
)

add_library(choqok_imageshack MODULE ${choqok_imageshack_PART_SRCS})

kcoreaddons_desktop_to_json(choqok_imageshack choqok_imageshack.desktop)

target_link_libraries(choqok_imageshack
PUBLIC
    Qt5::Core
    Qt5::Xml
    KF5::CoreAddons
    KF5::I18n
    KF5::KIOCore
    KF5::KDELibs4Support
    choqok
)

install(TARGETS choqok_imageshack DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES choqok_imageshack.desktop DESTINATION ${SERVICES_INSTALL_DIR})

add_subdirectory(icons)