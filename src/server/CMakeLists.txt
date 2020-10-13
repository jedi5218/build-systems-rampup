set(SERVER_INCLUDES
    server.h
)
set(SERVER_SOURCES
    main.cc
    server.cc
)
add_executable(server ${SERVER_INCLUDES} ${SERVER_SOURCES})
target_link_libraries(server PRIVATE common)
find_library(LIBPCAP_PATH pcap)
target_link_libraries(server PRIVATE ${LIBPCAP_PATH})