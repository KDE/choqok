
include_directories(
${CHOQOK_INCLUDES}
)

# link_directories(  )


########### next target ###############

set(kcm_choqok_appearanceconfig_PART_SRCS
appearanceconfig.cpp )

kde4_add_ui_files(kcm_choqok_appearanceconfig_PART_SRCS
appearanceconfig_base.ui )

kde4_add_plugin(kcm_choqok_appearanceconfig ${kcm_choqok_appearanceconfig_PART_SRCS})


target_link_libraries(kcm_choqok_appearanceconfig ${KDE4_KUTILS_LIBS} choqok)

install(TARGETS kcm_choqok_appearanceconfig  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES choqok_appearanceconfig.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

