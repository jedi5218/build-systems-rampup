#ifndef SERVER_H
#define SERVER_H

#include <condition_variable>
#include <cstring>
#include <thread>
#include <mutex>
#include <deque>
#include <pcap/pcap.h>

#include "thread.h"
#include "pcap_monitor.h"

#define PREFIX "Server recieved: "

class Server;
class ConnectionThread;

using ThreadRef = std::reference_wrapper<ConnectionThread>;

class Server
{
public:
    Server(short port);

    static const short fallback_port = 8080;
    static const short buffer_size = 1024;
    static const short backlog_size = 5;
    void run();

    PCapThread &pcap_monitor() { return pcap_thread; }

private:
    short port;
    void server_loop(int sockfd);
    ThreadPool connection_threads_pool;
    PCapThread pcap_thread;
};

class Connection : public ThreadStore
{
public:
    Connection(Server *parent);
    static Task make(int connfd);

private:
    Server *parent;
    bool echo(int connfd);
    const char *prefix = PREFIX;
    static const uint prefix_len = strlen(PREFIX);
    char write_buffer[Server::buffer_size + prefix_len] = {0};
    char *read_buffer = write_buffer + prefix_len;
};

#endif // SERVER
