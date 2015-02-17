#find_package(TelepathyQt4)

configure_file(config-imstatus.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-imstatus.h )

include_directories(
${TELEPATHY_QT4_INCLUDE_DIR}
${CHOQOK_INCLUDES}
)

########### next target ###############

set( choqok_imstatus_PART_SRCS imqdbus.cpp
imstatus.cpp )

kconfig_add_kcfg_files(choqok_imstatus_PART_SRCS imstatussettings.kcfgc )

add_library(choqok_imstatus MODULE ${choqok_imstatus_PART_SRCS})

target_link_libraries(choqok_imstatus KF5::KIOCore ${TELEPATHY_QT4_LIBRARIES} choqok)

install(TARGETS choqok_imstatus DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

set( kcm_choqok_imstatus_PART_SRCS imqdbus.cpp
imstatusconfig.cpp )

ki18n_wrap_ui(kcm_choqok_imstatus_PART_SRCS imstatusprefs.ui )

kconfig_add_kcfg_files(kcm_choqok_imstatus_PART_SRCS imstatussettings.kcfgc )

add_library(kcm_choqok_imstatus MODULE ${kcm_choqok_imstatus_PART_SRCS})

target_link_libraries(kcm_choqok_imstatus  ${TELEPATHY_QT4_LIBRARIES})

install(TARGETS kcm_choqok_imstatus DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

install( FILES imstatussettings.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES choqok_imstatus_config.desktop choqok_imstatus.desktop
         DESTINATION ${SERVICES_INSTALL_DIR})
