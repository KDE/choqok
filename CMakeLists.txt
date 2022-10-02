cmake_minimum_required(VERSION 3.16)

project(choqok)

set (QT_MIN_VERSION "5.15.0")
set (KF_MIN_VERSION "5.90.0")

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

include(ECMInstallIcons)
include(ECMQtDeclareLoggingCategory)
include(GenerateExportHeader)
include(FeatureSummary)

find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} CONFIG REQUIRED
    Core
    DBus
    Network
    NetworkAuth
    Widgets
)
find_package(KF5 ${KF_MIN_VERSION} REQUIRED
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

find_package(KF5 COMPONENTS Purpose)

#option(QTINDICATE_DISABLE "Disable support for notifications via indicator")

find_package(Qca-qt${QT_MAJOR_VERSION} REQUIRED)

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

ecm_qt_install_logging_categories(
    EXPORT CHOQOK
    FILE choqok.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)

ki18n_install( po )

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
