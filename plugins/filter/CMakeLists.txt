
include_directories(
${CHOQOK_INCLUDES}
)

set( choqok_filter_PART_SRCS
filter.cpp
configurefilters.cpp
filtersettings.cpp
filtermanager.cpp
addeditfilter.cpp )

ki18n_wrap_ui( choqok_filter_PART_SRCS addeditfilter_base.ui filterprefs.ui )

add_library(choqok_filter MODULE ${choqok_filter_PART_SRCS})

target_link_libraries(choqok_filter KF5::KIOCore choqok twitterapihelper)

install(TARGETS choqok_filter DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

install( FILES choqok_filter.desktop
         DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES filterui.rc  DESTINATION  ${DATA_INSTALL_DIR}/choqok_filter )
