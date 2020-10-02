#include "server.h"

#include <cerrno>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define SA struct sockaddr

Server::Server(int port) : port(port) {}

void Server::server_loop(int sockfd)
{
    const auto prefix = "Server recieved: ";
    const uint prefix_len = strlen(prefix);

    char write_buffer[BUFFER_SIZE + prefix_len] = {0};
    strcpy(write_buffer, prefix);
    char *read_buffer = write_buffer + prefix_len;

    int connfd;

    sockaddr_in cli;
    for (;;)
    {
        printf("Waiting for a new connection...\n");
        socklen_t len = sizeof(cli);
        connfd = accept(sockfd, (SA *)&cli, &len);
        if (connfd < 0)
        {
            printf("server accception failed...\n");
            return;
        }
        else
            printf("server acccepts the new client.\n");

        for (;;)
        {

            bzero(read_buffer, BUFFER_SIZE);
            int retval = read(connfd, read_buffer, BUFFER_SIZE);
            if (retval <= 0)
            {
                if (retval < 0)
                    printf("Unexpected error while recieving client query. id #%i: %s\n", errno, strerror(errno));
                close(connfd);
                printf("Closing connection.\n");
                break;
            }
            printf("Message to return: %s\n", write_buffer);
            retval = write(connfd, write_buffer, strlen(write_buffer));
            if (retval <= 0)
            {
                if (retval < 0)
                    printf("Unexpected error while sending the response. id #%i: %s\n", errno, strerror(errno));
                close(connfd);
                printf("Closing connection.\n");
                break;
            }
        }
    }
}

void Server::run()
{
    printf("Starting server on port %i\n", port);
    int sockfd;
    sockaddr_in servaddr;
    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        printf("socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    // Binding newly created socket to given IP and verification
    if ((bind(sockfd, (SA *)&servaddr, sizeof(servaddr))) != 0)
    {
        printf("socket bind failed. Cause: ");
        switch (errno)
        {
        case EACCES:
            printf("insufficient rights to acces the socket.\n");
            break;
        case EADDRINUSE:
            printf("port is already taken.\n");
            break;
        default:
            printf("unexpected error id #%i: %s\n", errno, strerror(errno));
            break;
        }
        exit(0);
    }
    else
        printf("Socket successfully binded.\n");

    if ((listen(sockfd, 5)) != 0)
    {
        printf("Listen failed.\n");
        exit(0);
    }
    else
        printf("Server listening..\n");

    server_loop(sockfd);
    close(sockfd);
}
