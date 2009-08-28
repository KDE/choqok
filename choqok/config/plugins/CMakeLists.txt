include_directories( 
${CHOQOK_INCLUDES}
)

########### next target ###############

set(kcm_choqok_pluginconfig_PART_SRCS choqokpluginconfig.cpp )

kde4_add_plugin(kcm_choqok_pluginconfig ${kcm_choqok_pluginconfig_PART_SRCS})


target_link_libraries(kcm_choqok_pluginconfig  ${KDE4_KUTILS_LIBS} choqok)

install(TARGETS kcm_choqok_pluginconfig DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES choqok_pluginconfig.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
