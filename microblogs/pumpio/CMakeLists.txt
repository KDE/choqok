include_directories(
    ${CHOQOK_INCLUDES}
    ${CMAKE_CURRENT_BINARY_DIR}
)

set(choqok_pumpio_SRCS
    pumpioaccount.cpp
    pumpiocomposerwidget.cpp
    pumpioeditaccountwidget.cpp
    pumpiomessagedialog.cpp
    pumpiomicroblog.cpp
    pumpiomicroblogwidget.cpp
    pumpiopost.cpp
    pumpiopostwidget.cpp
    pumpioshowthread.cpp
)

ki18n_wrap_ui(choqok_pumpio_SRCS
    pumpioeditaccountwidget.ui
    pumpiomessagedialog.ui
    pumpioshowthread.ui
)

add_library(choqok_pumpio MODULE ${choqok_pumpio_SRCS})

target_link_libraries(choqok_pumpio choqok
    KF5::KIOCore
    ${QTOAUTH_LIBRARY}
    ${QJSON_LIBRARY}
)

install(TARGETS choqok_pumpio DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES choqok_pumpio.desktop DESTINATION ${SERVICES_INSTALL_DIR})

add_subdirectory(icons)
