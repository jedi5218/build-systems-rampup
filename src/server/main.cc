#include <iostream>
#include <string>
#include <stdexcept>

#include "server.h"
#include "common/argsparser.h"

int main(int argc, char *argv[])
{
    Argument port_param('p', "port", true, "Port to listen on for client connections");
    short port = Server::fallback_port;
    if (!Argument::parse_arguments({port_param}, argc, argv, "The server application."))
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

    // PCapThread t;
    // t.run();
    // exit(0);

    Server server(port);
    server.run();
    return 0;
}