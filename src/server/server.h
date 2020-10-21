#ifndef SERVER_H
#define SERVER_H

#include <condition_variable>
#include <cstring>
#include <thread>
#include <mutex>
#include <deque>
#include <pcap/pcap.h>

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

private:
    short port;
    void server_loop(int sockfd);
    friend class ConnectionThread;
    std::mutex free_thread_queue_mutex, free_thread_cond_mutex;
    std::condition_variable connect_condition, free_thread_condition;
    std::deque<ConnectionThread *> free_threads;
    void push_free_thread(ConnectionThread *thread);
};

class ConnectionThread
{
public:
    ConnectionThread(Server &parent);
    void connect(int connfd);
    bool echo(int connfd);

private:
    std::thread thread;
    Server &parent;
    int connfd = 0;
    std::mutex connfd_mutex, free_thread_cond_mutex;
    void run();

    const char *prefix = PREFIX;
    static const uint prefix_len = strlen(PREFIX);
    char write_buffer[Server::buffer_size + prefix_len] = {0};
    char *read_buffer = write_buffer + prefix_len;
};

#endif // SERVER
