#include <iostream>
#include "argsparser.h"

int main(int argc, char *argv[])
{
    Argument port('p', "port", true);
    Argument ip_addr('i', "ip-addr", true);
    Argument cow('c', "cow", false);

    if (!Argument::parse_arguments({port, ip_addr, cow}, argc, argv, "The server application."))
        return 0;

    if (port.is_set())
        std::cout << "listening on the port" << port.value() << std::endl;
    else
        std::cout << "port was not provided, defaulting to 8080" << std::endl;
    return 0;
}