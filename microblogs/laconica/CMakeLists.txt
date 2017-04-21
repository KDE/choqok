
include_directories(
    ${CHOQOK_INCLUDES}
)

set(choqok_laconica_SRCS
    laconicadebug.cpp
    laconicaeditaccount.cpp
    laconicamicroblog.cpp
)

ki18n_wrap_ui(choqok_laconica_SRCS laconicaeditaccount_base.ui)

add_library(choqok_laconica MODULE ${choqok_laconica_SRCS})

kcoreaddons_desktop_to_json(choqok_laconica choqok_laconica.desktop)

target_link_libraries(choqok_laconica
PUBLIC
    Qt5::Core
    Qt5::NetworkAuth
    Qt5::Widgets
    KF5::CoreAddons
    KF5::I18n
    KF5::KIOCore
    KF5::KIOWidgets
    choqok
    gnusocialapihelper
)

install(TARGETS choqok_laconica DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES choqok_laconica.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

add_subdirectory(icons)
