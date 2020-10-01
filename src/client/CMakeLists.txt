set(CLIENT_SOURCES
    main.cc
)
add_executable(client ${CLIENT_SOURCES})
target_link_libraries(client PRIVATE common)
