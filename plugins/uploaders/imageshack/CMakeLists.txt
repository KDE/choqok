
include_directories(
${CHOQOK_INCLUDES}
)

add_subdirectory(icons)

########### next target ###############

set( choqok_imageshack_PART_SRCS
imageshack.cpp)

# kconfig_add_kcfg_files(choqok_imageshack_PART_SRCS imageshacksettings.kcfgc )

add_library(choqok_imageshack MODULE ${choqok_imageshack_PART_SRCS})

target_link_libraries(choqok_imageshack KF5::KIOCore choqok)

install(TARGETS choqok_imageshack DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

install( FILES choqok_imageshack.desktop
         DESTINATION ${SERVICES_INSTALL_DIR})