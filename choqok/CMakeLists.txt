
include_directories(
${CHOQOK_INCLUDES}
${CMAKE_CURRENT_BINARY_DIR}/config
${CMAKE_CURRENT_SOURCE_DIR}/config
)

set(choqok_SRCS
   mainwindow.cpp
   main.cpp
   systrayicon.cpp
   config/accountswidget.cpp
   config/advancedconfig.cpp
   config/addaccountdialog.cpp
   config/editaccountdialog.cpp
)

kde4_add_ui_files(choqok_SRCS config/prefs_base.ui config/accountswidget_base.ui
config/appears_base.ui config/advanced_base.ui)

kde4_add_kcfg_files(choqok_SRCS settings.kcfgc )

kde4_add_app_icon(choqok_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/../icons/hi*-app-choqok.png")

kde4_add_executable(choqok_bin ${choqok_SRCS})
set_target_properties(choqok_bin PROPERTIES OUTPUT_NAME choqok)

target_link_libraries(choqok_bin ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} choqok)

install(TARGETS choqok_bin ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install( FILES choqok.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES choqok.kcfg  DESTINATION  ${KCFG_INSTALL_DIR} )
install( FILES choqokui.rc choqok.notifyrc  DESTINATION  ${DATA_INSTALL_DIR}/choqok )

