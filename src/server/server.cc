#include "server.h"

#include <cstring>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <pcap.h>
#include <sys/socket.h>
#include <unistd.h>
#include <thread>

#include <chrono>

#include <common/utils.h>

Server::Server(short port) : port(port)
{
    for (uint i = 0; i < std::thread::hardware_concurrency(); i++)
        free_threads.push_front(new ConnectionThread(*this));
}

void Server::server_loop(int sockfd)
{
    int connfd;
    sockaddr_in cli;
    for (;;)
    {
        printf("Waiting for a new connection...\n");
        socklen_t len = sizeof(cli);
        if (free_threads.empty())
        {
            std::unique_lock<std::mutex> guard(free_thread_cond_mutex);
            free_thread_condition.wait(guard, [this]() -> bool { !this->free_threads.empty(); });
        }

        connfd = handle_posix_error(
            accept(
                sockfd,
                reinterpret_cast<sockaddr *>(&cli),
                &len),
            "server acception");
        char client_address_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(cli.sin_addr), client_address_str, INET_ADDRSTRLEN);
        printf("Accepted new client at %s:%d\n", client_address_str, ntohs(cli.sin_port));

        {
            std::lock_guard<std::mutex> guard(free_thread_queue_mutex);
            free_threads.front()->connect(connfd);
            free_threads.pop_front();
        }
    }
}

void Server::run()
{
    printf("Starting server on port %i\n", port);
    int sockfd;
    sockaddr_in servaddr;

    sockfd = handle_posix_error(
        socket(AF_INET, SOCK_STREAM, 0), "Socket creation");
    printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    handle_posix_error(
        bind(sockfd, reinterpret_cast<sockaddr *>(&servaddr), sizeof(servaddr)), "Socket binding");
    printf("Socket successfully binded.\n");

    handle_posix_error(
        listen(sockfd, backlog_size), "Socket listen");
    printf("Server listening..\n");

    server_loop(sockfd);

    close(sockfd);
}

void Server::push_free_thread(ConnectionThread *thread)
{
    std::lock_guard<std::mutex> guard(free_thread_queue_mutex);
    free_threads.push_back(thread);
}

ConnectionThread::ConnectionThread(Server &parent) : parent(parent),
                                                     thread(&ConnectionThread::run, this)
{
    strcpy(write_buffer, prefix);
}
void ConnectionThread::connect(int connfd)
{
    std::lock_guard<std::mutex> guard(connfd_mutex);
    this->connfd = connfd;
    parent.connect_condition.notify_all();
}

void ConnectionThread::run()
{
    std::cout << "Thread id " << std::hex << std::this_thread::get_id() << " started." << std::endl;
    for (;;)
    {
        std::unique_lock<std::mutex> guard(free_thread_cond_mutex);
        parent.connect_condition.wait(guard, [this]() -> bool { return this->connfd; });
        std::cout << "Thread id " << std::hex << std::this_thread::get_id()
                  << " got assigned a connection." << std::endl;
        while (echo(connfd))
            ;
        close(connfd);
        connfd = 0;
        parent.push_free_thread(this);
        parent.free_thread_condition.notify_all();
        std::cout << "Thread id " << std::hex << std::this_thread::get_id() << " freed." << std::endl;
    }
}

bool ConnectionThread::echo(int connfd)
{
    char *dev, errbuf[PCAP_ERRBUF_SIZE];
    pcap_if_t *devs;
    handle_posix_error(pcap_findalldevs(&devs, errbuf), "Default device lookup");
    dev = devs->name;

    bzero(read_buffer, Server::buffer_size);
    int retval = read(connfd, read_buffer, Server::buffer_size);
    if (retval <= 0)
    {
        if (retval < 0)
            printf("Unexpected error while recieving client query. id #%i: %s\n", errno, strerror(errno));
        close(connfd);
        printf("Closing connection.\n");
        return false;
    }
    std::cout << "Responding to client in thread " << std::hex << std::this_thread::get_id() << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(10));
    printf("Message to return: %s\n", write_buffer);
    strcpy(write_buffer + strlen(write_buffer), " default device: ");
    strcpy(write_buffer + strlen(write_buffer), dev);
    retval = write(connfd, write_buffer, strlen(write_buffer));
    if (retval <= 0)
    {
        if (retval < 0)
            printf("Unexpected error while sending the response. id #%i: %s\n", errno, strerror(errno));
        close(connfd);
        printf("Closing connection.\n");
        return false;
    }
    return true;
}
