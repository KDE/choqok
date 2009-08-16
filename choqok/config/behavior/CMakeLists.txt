
include_directories(
${CHOQOK_INCLUDES}
)


########### next target ###############

set(kcm_choqok_behaviorconfig_PART_SRCS
   behaviorconfig.cpp
   behaviorconfig_shorten.cpp
   )

kde4_add_ui_files(kcm_choqok_behaviorconfig_PART_SRCS
   behaviorconfig_general.ui
   behaviorconfig_notifications.ui
   behaviorconfig_shorten_base.ui
   )


kde4_add_plugin(kcm_choqok_behaviorconfig ${kcm_choqok_behaviorconfig_PART_SRCS})


target_link_libraries(kcm_choqok_behaviorconfig  ${KDE4_KUTILS_LIBS} choqok)

install(TARGETS kcm_choqok_behaviorconfig  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES choqok_behaviorconfig.desktop  DESTINATION ${SERVICES_INSTALL_DIR})


