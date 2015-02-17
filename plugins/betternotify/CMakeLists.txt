
include_directories(
${CHOQOK_INCLUDES}
)

########### next target ###############

set( choqok_notify_PART_SRCS mytextbrowser.cpp
notification.cpp
notify.cpp
notifysettings.cpp )

# kconfig_add_kcfg_files(choqok_notify_PART_SRCS betternotify.kcfgc )

add_library(choqok_betternotify MODULE ${choqok_notify_PART_SRCS})

target_link_libraries(choqok_betternotify   choqok)

install(TARGETS choqok_betternotify DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

set( kcm_choqok_notify_PART_SRCS
dummynotification.cpp
notifyconfig.cpp
notifysettings.cpp )

ki18n_wrap_ui(kcm_choqok_notify_PART_SRCS notifyprefs.ui )

# kconfig_add_kcfg_files(kcm_choqok_notify_PART_SRCS betternotify.kcfgc )

add_library(kcm_choqok_betternotify MODULE ${kcm_choqok_notify_PART_SRCS})

target_link_libraries(kcm_choqok_betternotify  choqok)

install(TARGETS kcm_choqok_betternotify DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

install( FILES choqok_notify.desktop choqok_notify_config.desktop
         DESTINATION ${SERVICES_INSTALL_DIR})
