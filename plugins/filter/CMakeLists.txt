
include_directories(
${CHOQOK_INCLUDES}
)

set( choqok_filter_PART_SRCS
filter.cpp
configurefilters.cpp
filtersettings.cpp
filtermanager.cpp
addeditfilter.cpp )

kde4_add_ui_files( choqok_filter_PART_SRCS addeditfilter_base.ui filterprefs.ui )

kde4_add_plugin(choqok_filter ${choqok_filter_PART_SRCS})

target_link_libraries(choqok_filter ${KDE4_KIO_LIBS} choqok twitterapihelper)

install(TARGETS choqok_filter DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

install( FILES choqok_filter.desktop
         DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES filterui.rc  DESTINATION  ${DATA_INSTALL_DIR}/choqok_filter )
