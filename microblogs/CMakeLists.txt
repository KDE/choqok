find_package(KF5Attica)

add_subdirectory(twitter)
add_subdirectory(mastodon)
add_subdirectory(laconica)
add_subdirectory(friendica)
add_subdirectory(pumpio)

IF(KF5Attica_FOUND)
    message(STATUS "FOUND LibAttica: Will build \"Open Collaboration Services\" plugin")
    add_subdirectory(ocs)
ELSE(KF5Attica_FOUND)
    message(WARNING "Optional dependency \"LibAttica\" NOT FOUND, won't build \"Open Collaboration Services\" plugin")
ENDIF(KF5Attica_FOUND)
