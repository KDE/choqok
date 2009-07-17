
include_directories(
${CHOQOK_INCLUDES}
)

set(choqok_LIB_SRCS
plugin.cpp
shortener.cpp
pluginmanager.cpp
shortenmanager.cpp
)

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

install(TARGETS choqok ${INSTALL_TARGETS_DEFAULT_ARGS})
install( FILES choqokplugin.desktop choqokshortenerplugin.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})
