#include <iostream>
#include <string>
#include <stdexcept>

#include "server.h"
#include "argsparser.h"

int main(int argc, char *argv[])
{
    Argument port_param('p', "port", true);
    int port = 8080;
    if (!Argument::parse_arguments({port_param}, argc, argv, "The server application."))
        return 0;
    if (port_param.is_set())
        try
        {
            port = std::stoi(port_param.value());
        }
        catch (const std::invalid_argument &e)
        {
            return 0;
        }
    else
        std::cout << "port was not provided, defaulting to 8080" << std::endl;
    Server server(port);
    server.run();
    return 0;
}