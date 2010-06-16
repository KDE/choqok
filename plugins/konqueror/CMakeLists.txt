#include(DBusMacros)

include(MacroOptionalDependPackage)

MACRO_OPTIONAL_DEPEND_PACKAGE(KWebKitPart "kwebkitpart")
if(DEPEND_PACKAGE_KWebKitPart)
	#macro_optional_find_package(KWebKitPart QUIET)
	find_package(KWebKitPart)
else(DEPEND_PACKAGE_KWebKitPart)
	set(KWEBKITPART_FOUND FALSE)
endif(DEPEND_PACKAGE_KWebKitPart)

if(KWEBKITPART_FOUND)
	add_definitions(-DHAVE_KWEBKITPART)
	include_directories(${KWEBKITPART_INCLUDE_DIR})
	set(HAVE_KWEBKITPART true)
endif(KWEBKITPART_FOUND)


macro_log_feature(KWEBKITPART_FOUND "KWebKitPart" "KWebKitPart libraries and header files" "https://svn.kde.org/home/kde/trunk/extragear/base/kwebkitpart" FALSE "" "KWebKitPart is necessary to compile Choqok support for KWebKitPart.")

include_directories(
${CHOQOK_INCLUDES}
)

set(konqchoqokplugin_SRCS konqchoqok.cpp)

kde4_add_plugin(konqchoqokplugin ${konqchoqokplugin_SRCS})

target_link_libraries(konqchoqokplugin ${KDE4_KHTML_LIBS} choqok )

if(KWEBKITPART_FOUND)
	target_link_libraries(konqchoqokplugin ${KWEBKITPART_LIBRARIES} choqok ${KDE4_KDEWEBKIT_LIBS} ${QT_QTWEBKIT_LIBRARY})
endif(KWEBKITPART_FOUND)

install(TARGETS konqchoqokplugin DESTINATION ${PLUGIN_INSTALL_DIR} )

install(FILES konqchoqok.rc konqchoqok.desktop DESTINATION ${DATA_INSTALL_DIR}/khtml/kpartplugins )

if(KWEBKITPART_FOUND)
	install(FILES konqchoqok.rc konqchoqok.desktop DESTINATION ${DATA_INSTALL_DIR}/kwebkitpart/kpartplugins )
endif (KWEBKITPART_FOUND)

install( FILES choqok_share.desktop DESTINATION ${SERVICES_INSTALL_DIR}/ServiceMenus )
install( FILES choqok_upload.desktop DESTINATION ${SERVICES_INSTALL_DIR}/ServiceMenus )
