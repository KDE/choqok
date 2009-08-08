
include_directories(
${CHOQOK_INCLUDES}
)

set(choqok_LIB_SRCS
plugin.cpp
shortener.cpp
account.cpp
microblog.cpp
pluginmanager.cpp
shortenmanager.cpp
accountmanager.cpp
passwordmanager.cpp
mediamanager.cpp
ui/microblogwidget.cpp
ui/editaccountwidget.cpp
ui/timelinewidget.cpp
ui/postwidget.cpp
ui/choqoktextedit.cpp
ui/composerwidget.cpp
ui/quickpost.cpp
# filtermanager.cpp
# filter.cpp
)
kde4_add_kcfg_files(choqok_LIB_SRCS
choqokbehaviorsettings.kcfgc
choqokappearancesettings.kcfgc
)
# kde4_add_ui_files(choqok_LIB_SRCS ui/timelinewidget_base.ui )

kde4_add_library(choqok SHARED ${choqok_LIB_SRCS})

target_link_libraries(choqok ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KUTILS_LIBS})

set_target_properties(choqok PROPERTIES
VERSION 0.6.5
SOVERSION 0
)
if(MSVC)
    set_target_properties(choqok PROPERTIES
    OUTPUT_NAME libchoqok
    )
endif(MSVC)

install( FILES choqokbehaviorsettings.kcfg choqokappearancesettings.kcfg
         DESTINATION ${KCFG_INSTALL_DIR})

install(TARGETS choqok ${INSTALL_TARGETS_DEFAULT_ARGS})
install( FILES choqokplugin.desktop choqokshortenerplugin.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})

