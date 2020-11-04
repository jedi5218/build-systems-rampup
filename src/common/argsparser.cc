#include "argsparser.h"

Argument::Argument(char key, const char *long_name,
                   bool has_parameter, const char *description) : long_name_str(long_name),
                                                                  key_char(key),
                                                                  has_param(has_parameter),
                                                                  description(description)
{
}

bool Argument::parse_arguments(ArgumentList arguments, int argc, char *argv[], const char *help_text)
{
    auto args_map = make_args_dict(arguments);
    auto short_options = make_short_opts_string(arguments);
    auto long_options = make_options_list(arguments);
    bool error = false;

    char c;
    while ((c = getopt_long(argc, argv, short_options.c_str(), long_options.get(), nullptr)) != -1)
    {
        if (args_map.count(c))
        {
            auto &arg = args_map.at(c).get();
            if (arg.has_parameter())
            {
                arg.value_str = std::string(optarg);
            }
            arg.set = true;
        }
        else
        {

            switch (c)
            {
            case 'h':
                printf(
                    "%s\n%s\n"
                    "\nUSAGE:\n"
                    "%s [OPTIONS]\n"
                    "\nOPTIONS:\n",
                    argv[0], help_text, argv[0]);
                for (auto arg : arguments)
                {
                    printf(
                        "\t-%c --%s: %s\n",
                        arg.get().key(),
                        arg.get().long_name_str,
                        arg.get().description);
                }
                error = true;
                break;

            case ':':
                fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                error = true;
                break;

            case '?':
                if (optopt == 'c')
                    fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                else if (isprint(optopt))
                    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                else if (optopt)
                    fprintf(stderr,
                            "Unknown option character `\\x%x'.\n",
                            optopt);
                else
                    fprintf(stderr,
                            "Parameter '%s' is not recognised.\n",
                            argv[optind - 1]);
                error = true;
                break;

            default:
                error = true;
                break;
            }
            if (error)
                break;
        }
    }
    return !error;
}

ArgumentDict Argument::Argument::make_args_dict(const ArgumentList &arguments)
{
    ArgumentDict arg_dict;
    for (auto arg_ref : arguments)
    {
        auto &arg = arg_ref.get();
        arg_dict.emplace(std::make_pair<char, std::reference_wrapper<Argument>>(arg.key(), arg));
    }
    return arg_dict;
}

std::string Argument::Argument::make_short_opts_string(const ArgumentList &arguments)
{
    std::string short_options = ":h";
    for (auto arg_ref : arguments)
    {
        auto &arg = arg_ref.get();
        short_options += arg.key();
        if (arg.has_parameter())
            short_options += ':';
    }
    return short_options;
}

std::unique_ptr<option[]> Argument::Argument::make_options_list(const ArgumentList &arguments)
{
    std::unique_ptr<option[]> long_options(new option[arguments.size() + 2]);
    long_options[arguments.size()] =
        {"help", no_argument, nullptr, 'h'};

    long_options[arguments.size() + 1] =
        {0, 0, 0, 0};

    uint i = 0;
    for (auto arg_ref : arguments)
    {
        auto argument = arg_ref.get();
        long_options[i++] =
            {argument.long_name_str,
             argument.has_parameter() ? required_argument : no_argument,
             nullptr,
             argument.key()};
    }

    return long_options;
}
