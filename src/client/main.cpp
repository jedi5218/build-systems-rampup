#include <iostream>
#include "argsparser.hpp"
int main(int argc, char *argv[])
{
    Argument port('p', "port", true);
    Argument ip_addr('i',"ip-addr",true);
    Argument cow('c', "cow", false);
    ArgsParser parser(
        {
            port,
            ip_addr,
            cow
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