#include "server.h"

#include <chrono>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

#include <common/utils.h>

Server::Server(short port) : port(port),
                             connection_threads_pool(std::thread::hardware_concurrency(),
                                                     [this]() { return new Connection(this); })
{
    connection_threads_pool.start();
    pcap_thread.start();
}

void Server::server_loop(int sockfd)
{
    int connfd;
    sockaddr_in cli;
    for (;;)
    {
        printf("Waiting for a new connection...\n");
        socklen_t len = sizeof(cli);
        connfd = handle_posix_error(
            accept(
                sockfd,
                reinterpret_cast<sockaddr *>(&cli),
                &len),
            "server acception");
        char client_address_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(cli.sin_addr), client_address_str, INET_ADDRSTRLEN);
        printf("Accepted new client at %s:%d\n", client_address_str, ntohs(cli.sin_port));
        connection_threads_pool.push_task(Connection::make(connfd));
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

Connection::Connection(Server *parent) : parent(parent)
{
    strcpy(write_buffer, prefix);
    read_buffer = write_buffer + prefix_len;
}

Task Connection::make(int connfd)
{
    return [connfd](ThreadStore &store) {
        auto connection = static_cast<Connection &>(store);
        while (connection.echo(connfd))
            ;
        close(connfd);
    };
}

bool Connection::echo(int connfd)
{

    bzero(write_buffer + prefix_len, Server::buffer_size);
    int char_count = read(connfd, write_buffer + prefix_len, Server::buffer_size);
    if (char_count <= 0)
    {
        if (char_count < 0)
            printf("Unexpected error while recieving client query. id #%i: %s\n", errno, strerror(errno));
        close(connfd);
        printf("Closing connection.\n");
        return false;
    }
    std::cout << "Responding to client in thread " << std::hex << std::this_thread::get_id() << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));
    if (strcmp(write_buffer + prefix_len, "report") == 0)
    {
        strcpy(write_buffer + strlen(write_buffer),
               parent->pcap_monitor().connections().compile_report().c_str());
    }
    printf("Message to return: %s\n", write_buffer);
    strcpy(write_buffer + strlen(write_buffer), read_buffer);
    char_count = write(connfd, write_buffer, strlen(write_buffer));
    if (char_count <= 0)
    {
        if (char_count < 0)
            printf("Unexpected error while sending the response. id #%i: %s\n", errno, strerror(errno));
        close(connfd);
        printf("Closing connection.\n");
        return false;
    }
    return true;
}
