#ifndef SERVER_H
#define SERVER_H

class Server
{
public:
    Server(int port = 8080);
    void run();

protected:
    int port;
    void server_loop(int sockfd);
};

#endif //SERVER
