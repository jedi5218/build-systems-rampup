set(SERVER_INCLUDES
    server.h
    thread.h
    pcap_monitor.h
)
set(SERVER_SOURCES
    main.cc
    server.cc
    thread.cc
    pcap_monitor.cc
)
add_executable(server ${SERVER_INCLUDES} ${SERVER_SOURCES})
target_link_libraries(server PRIVATE common)
target_link_libraries(server PRIVATE pthread)
find_library(LIBPCAP_PATH pcap)
target_link_libraries(server PRIVATE ${LIBPCAP_PATH})