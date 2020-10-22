#include <arpa/inet.h>
#include <cctype>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <getopt.h>
#include <iostream>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include "common/argsparser.h"

int main(int argc, char *argv[])
{

    Argument ip_addr('a', "server-addr", true, "Server ip-address");
    Argument port_param('p', "server-port", true, "Port of the server");
    Argument echo('s', "string", true, "Message to send to the server");
    Argument interactive('i', "interacrive", false, "Run client in interactive mode");

    short port = 8080;
    if (!Argument::parse_arguments({ip_addr, port_param, echo, interactive}, argc, argv, "The client application."))
        return 0;
    if (port_param.is_set())
        try
        {
            port = std::stoi(port_param.value());
        }
        catch (const std::invalid_argument &e)
        {
            std::cerr << "Port parameter must be a number" << std::endl;
            return 0;
        }
        catch (const std::exception &e)
        {
            std::cerr
                << "Error encountered while reading port number: "
                << e.what() << std::endl;
            return 1;
        }
    else
        std::cout << "port was not provided, defaulting to " << port << std::endl;

    int sockfd;
    struct sockaddr_in servaddr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        printf("Socket creation failed. %s\n", strerror(errno));
        exit(0);
    }
    else
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(ip_addr.value().c_str());
    servaddr.sin_port = htons(port);

    // connect the client socket to server socket
    if (connect(sockfd, reinterpret_cast<sockaddr *>(&servaddr), sizeof(servaddr)) != 0)
    {
        printf("connection with the server failed. %s\n", strerror(errno));
        exit(0);
    }
    else
        printf("connected to the server..\n");
    char buff[256];
    bzero(buff, sizeof(buff));
    memcpy(buff, echo.value().c_str(), sizeof(buff) - 1);
    int retval = write(sockfd, buff, sizeof(buff));
    if (retval <= 0)
    {
        if (retval < 0)
            printf("Unexpected error while sending. id #%i: %s\n", errno, strerror(errno));
        printf("Connection with server was closed before the query was sent.\n");
        close(sockfd);
        return 0;
    }
    bzero(buff, sizeof(buff));
    retval = read(sockfd, buff, sizeof(buff));
    if (retval <= 0)
    {
        if (retval < 0)
            printf("Unexpected error while recieving. id #%i: %s\n", errno, strerror(errno));
        printf("Connection with server was closed before the response was recieved.\n");
        close(sockfd);
        return 0;
    }
    printf("Processed string: %s\n", buff);
    close(sockfd);
    return 0;
}