set(SERVER_SOURCES
    main.cc
)
add_executable(server ${SERVER_SOURCES})
target_link_libraries(server PRIVATE common)
