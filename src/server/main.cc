#include <iostream>
#include "argsparser.h"
int main(int argc, char *argv[])
{
    Argument port('p', "port", true);
    ArgsParser parser(
        {
            port,
        },
        "The server application.");
    if (parser.parse(argc, argv))
    {
        if (port.is_set())
            std::cout << "listening on the port" << port.value() << std::endl;
        else
            std::cout << "port was not provided, defaulting to 8080" << std::endl;
    }
}