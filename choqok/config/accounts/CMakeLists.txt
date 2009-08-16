

include_directories(
${CHOQOK_INCLUDES}
)

# link_directories(  )

########### next target ###############

set(kcm_choqok_accountsconfig_PART_SRCS
accountswidget.cpp
addaccountdialog.cpp
editaccountdialog.cpp )


kde4_add_ui_files(kcm_choqok_accountsconfig_PART_SRCS accountswidget_base.ui )

kde4_add_plugin(kcm_choqok_accountsconfig ${kcm_choqok_accountsconfig_PART_SRCS})


target_link_libraries(kcm_choqok_accountsconfig ${KDE4_KDEUI_LIBS} ${KDE4_KUTILS_LIBS} choqok)

install(TARGETS kcm_choqok_accountsconfig  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES choqok_accountsconfig.desktop  DESTINATION ${SERVICES_INSTALL_DIR})


