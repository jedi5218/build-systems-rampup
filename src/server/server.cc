#include "server.h"

#include <cstring>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <pcap.h>
#include <sys/socket.h>
#include <unistd.h>

#include <common/utils.h>

Server::Server(short port) : port(port)
{
    strcpy(write_buffer, prefix);
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

        while (echo(connfd))
            ;
    }
}

bool Server::echo(int connfd)
{
    char *dev, errbuf[PCAP_ERRBUF_SIZE];
    pcap_if_t *devs;
    handle_posix_error(pcap_findalldevs(&devs, errbuf), "Default device lookup");
    dev = devs->name;

    bzero(read_buffer, buffer_size);
    int retval = read(connfd, read_buffer, buffer_size);
    if (retval <= 0)
    {
        if (retval < 0)
            printf("Unexpected error while recieving client query. id #%i: %s\n", errno, strerror(errno));
        close(connfd);
        printf("Closing connection.\n");
        return false;
    }
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
