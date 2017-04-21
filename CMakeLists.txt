project(choqok)

cmake_minimum_required(VERSION 2.8.12)

find_package(ECM REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

include(KDEInstallDirs)
include(ECMInstallIcons)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(FeatureSummary)

find_package(Qt5 CONFIG REQUIRED
    Core
    DBus
    Network
    NetworkAuth
    Widgets
)
find_package(KF5 REQUIRED
    ConfigWidgets
    CoreAddons
    DocTools
    I18n
    Emoticons
    GlobalAccel
    GuiAddons
    KCMUtils
    KIO
    Notifications
    NotifyConfig
    Sonnet
    TextWidgets
    Wallet
    WidgetsAddons
    XmlGui
)

#option(QTINDICATE_DISABLE "Disable support for notifications via indicator")

find_package(Qca-qt5 REQUIRED)

#if(NOT QTINDICATE_DISABLE)
#    find_package(QtIndicate)
#    if(QtIndicate_FOUND)
#        set(QTINDICATE_BUILD TRUE)
#	endif(QtIndicate_FOUND)
#endif(NOT QTINDICATE_DISABLE)

#if(QTINDICATE_BUILD)
#    message(STATUS "Found libindicate-qt: Will support notifications via indicator")
#else(QTINDICATE_BUILD)
#    message(STATUS "Optional dependency libindicate-qt not found: Won't support notifications via indicator")
#endif(QTINDICATE_BUILD)

set(CHOQOK_INCLUDES
${CMAKE_CURRENT_SOURCE_DIR}/libchoqok/
${CMAKE_CURRENT_SOURCE_DIR}/libchoqok/ui
)

add_subdirectory( libchoqok )
add_subdirectory( helperlibs )
add_subdirectory( choqok )
add_subdirectory( icons )
add_subdirectory( plugins )
add_subdirectory( microblogs )
add_subdirectory( images )
add_subdirectory( cmake )

include(ECMOptionalAddSubdirectory)
ecm_optional_add_subdirectory( doc )

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
