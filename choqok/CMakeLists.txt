set(choqok_SRCS
   search.cpp
   twittersearch.cpp
   identicasearch.cpp
   accounts.cpp
   accountswizard.cpp
   mainwindow.cpp
   main.cpp
   statuswidget.cpp
   backend.cpp
   statustextedit.cpp
   mediamanager.cpp
   systrayicon.cpp
   quicktwit.cpp
   timelinewidget.cpp
   searchwindow.cpp
   accountmanager.cpp
   account.cpp
   userinfowidget.cpp
   showthread.cpp
)
include_directories(
 ${CHOQOK_INCLUDES}
)
kde4_add_ui_files(choqok_SRCS timelinewidget_base.ui prefs_base.ui accounts_base.ui 
accounts_wizard_base.ui appears_base.ui quicktwit_base.ui searchwidget_base.ui
showthread_base.ui)

kde4_add_kcfg_files(choqok_SRCS settings.kcfgc )

kde4_add_app_icon(choqok_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/../icons/hi*-app-choqok.png")

kde4_add_executable(choqok_bin ${choqok_SRCS})
set_target_properties(choqok_bin PROPERTIES OUTPUT_NAME choqok)

target_link_libraries(choqok_bin ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KUTILS_LIBS})

install(TARGETS choqok_bin ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install( FILES choqok.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES choqok.kcfg  DESTINATION  ${KCFG_INSTALL_DIR} )
install( FILES choqokui.rc choqok.notifyrc  DESTINATION  ${DATA_INSTALL_DIR}/choqok )

