
include_directories(
${CHOQOK_INCLUDES}
)

add_subdirectory(icons)

########### next target ###############

set( choqok_imageshack_PART_SRCS
imageshack.cpp)

# kde4_add_kcfg_files(choqok_imageshack_PART_SRCS imageshacksettings.kcfgc )

kde4_add_plugin(choqok_imageshack ${choqok_imageshack_PART_SRCS})

target_link_libraries(choqok_imageshack ${KDE4_KIO_LIBS} choqok)

install(TARGETS choqok_imageshack DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

install( FILES choqok_imageshack.desktop
         DESTINATION ${SERVICES_INSTALL_DIR})