
include_directories(
${CHOQOK_INCLUDES}
)

########### next target ###############

set( choqok_notify_PART_SRCS mytextbrowser.cpp
notification.cpp
notify.cpp
notifysettings.cpp )

# kde4_add_kcfg_files(choqok_notify_PART_SRCS betternotify.kcfgc )

kde4_add_plugin(choqok_betternotify ${choqok_notify_PART_SRCS})

target_link_libraries(choqok_betternotify ${KDE4_KUTILS_LIBS} ${KDE4_KDEUI_LIBS} choqok)

install(TARGETS choqok_betternotify DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

set( kcm_choqok_notify_PART_SRCS
notifyconfig.cpp
notifysettings.cpp )

kde4_add_ui_files(kcm_choqok_notify_PART_SRCS notifyprefs.ui )

# kde4_add_kcfg_files(kcm_choqok_notify_PART_SRCS betternotify.kcfgc )

kde4_add_plugin(kcm_choqok_betternotify ${kcm_choqok_notify_PART_SRCS})

target_link_libraries(kcm_choqok_betternotify ${KDE4_KDEUI_LIBS} choqok)

install(TARGETS kcm_choqok_betternotify DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

install( FILES choqok_notify.desktop choqok_notify_config.desktop
         DESTINATION ${SERVICES_INSTALL_DIR})
