add_library(gnusocialapihelper SHARED)
set_target_properties(gnusocialapihelper PROPERTIES
    VERSION 1.3.0
    SOVERSION 1
)

generate_export_header(gnusocialapihelper)

ecm_qt_declare_logging_category(gnusocialapihelper
    HEADER gnusocialapidebug.h
    IDENTIFIER CHOQOK
    CATEGORY_NAME org.kde.choqok.gnusocialapi
    DESCRIPTION "choqok gnusocialapi library"
    EXPORT CHOQOK
)

target_sources(gnusocialapihelper PRIVATE
    gnusocialapiaccount.cpp
    gnusocialapicomposerwidget.cpp
    gnusocialapiconversationtimelinewidget.cpp
    gnusocialapidmessagedialog.cpp
    gnusocialapimicroblog.cpp
    gnusocialapipostwidget.cpp
    gnusocialapisearch.cpp
)

target_link_libraries(gnusocialapihelper
PUBLIC
    Qt::Core
    Qt::NetworkAuth
    Qt::Xml
    Qt::Widgets
    KF5::CoreAddons
    KF5::I18n
    KF5::KIOCore
    KF5::KIOWidgets
    KF5::WidgetsAddons
    choqok
    twitterapihelper
)

install(TARGETS gnusocialapihelper ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES
    gnusocialapiaccount.h
    gnusocialapicomposerwidget.h
    gnusocialapiconversationtimelinewidget.h
    gnusocialapidmessagedialog.h
    gnusocialapimicroblog.h
    gnusocialapipostwidget.h
    gnusocialapisearch.h
    ${CMAKE_CURRENT_BINARY_DIR}/gnusocialapihelper_export.h
DESTINATION ${KDE_INSTALL_INCLUDEDIR}/choqok/gnusocialapihelper COMPONENT Devel)
