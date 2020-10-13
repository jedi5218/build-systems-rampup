#ifndef SERVER_H
#define SERVER_H

#include <cstring>

#include <pcap/pcap.h>

#define PREFIX "Server recieved: "

class Server
{
public:
    Server(short port);

    static const short fallback_port = 8080;
    static const short buffer_size = 1024;
    static const short backlog_size = 5;
    void run();
    bool echo(int connfd);

private:
    short port;
    void server_loop(int sockfd);
    const char *prefix = PREFIX;
    static const uint prefix_len = strlen(PREFIX);
    char write_buffer[buffer_size + prefix_len] = {0};
    char *read_buffer = write_buffer + prefix_len;
};

#endif //SERVER
