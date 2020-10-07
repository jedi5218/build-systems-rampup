#include "utils.h"

#include <cerrno>
#include <cstring>
#include <iostream>

using std::cerr, std::endl;
int handle_posix_error(int value_to_verify, const char *operation)
{
    if (value_to_verify == -1)
    {
        cerr
            << operation << " failed." << endl
            << "error code: " << errno << endl
            << "'" << strerror(errno) << "'";
        exit(0);
    }
    else
        return value_to_verify;
}
