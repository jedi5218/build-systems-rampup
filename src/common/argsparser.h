#ifndef ARGSPARSER_H
#define ARGSPARSER_H

#include <functional>
#include <getopt.h>
#include <initializer_list>
#include <memory>
#include <string>
#include <unistd.h>
#include <unordered_map>

class Argument;

using ArgumentList = std::initializer_list<std::reference_wrapper<Argument>>;
using ArgumentDict = std::unordered_map<char, std::reference_wrapper<Argument>>;

class Argument
{
public:
    Argument(char key, const char *long_name, bool has_value, const char *description);
    bool has_parameter() const { return has_param; }
    bool is_set() const { return set; }
    std::string value() const { return value_str; }
    inline std::string long_name() const { return long_name_str; }
    inline char key() const { return key_char; }

    static bool parse_arguments(ArgumentList arguments, int argc, char *argv[], const char *help_text);

protected:
    const char *long_name_str;
    const char *description;
    const char key_char;
    const bool has_param;
    bool set = false;
    std::string value_str;
    static ArgumentDict make_args_dict(const ArgumentList &arguments);
    static std::string make_short_opts_string(const ArgumentList &arguments);
    static std::unique_ptr<option[]> make_options_list(const ArgumentList &arguments);
};

#endif //ARGSPARSER_H
