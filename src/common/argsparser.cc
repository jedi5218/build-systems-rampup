#include "argsparser.h"
#include <unistd.h>
#include <getopt.h>

#include <iostream>

ArgsParser::ArgsParser(ArgumentList arguments,const char* help):
    help_text(help)
{
    short_options = ":h";
    for (auto arg_ref : arguments)
    {
        auto& arg = arg_ref.get();
        arguments_.emplace(std::make_pair<char,std::reference_wrapper<Argument>>(arg.key(),arg));
        short_options += arg.key();
        if (arg.has_parameter())
            short_options += ':';
    }
}

bool ArgsParser::parse(int argc, char *argv[])
{
    option long_options[arguments_.size() + 2];
    long_options[arguments_.size()] = 
        {"help",no_argument,nullptr,'h'};
    long_options[arguments_.size() + 1] =
        {0, 0, 0, 0};
    uint i = 0;
    for (auto dict_item : arguments_)
    {
        auto argument = dict_item.second.get();
        long_options[i++] =
            {
                argument.long_name().c_str(),
                argument.has_parameter() ? required_argument : no_argument,
                nullptr,
                argument.key()};
    }
    char c;
    while ((c = getopt_long(argc, argv, short_options.c_str(), long_options, nullptr)) != -1)
    {
        if (arguments_.count(c))
        {
            auto& arg = arguments_.at(c).get();
            if(arg.has_parameter()){
                arg.value_ = std::string(optarg);
            }
            arg.is_set_ = true;
        }
        else
        {
            switch (c)
            {
            case 'h':
                puts(help_text);
                break;
            case ':':
                fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                return false;
            case '?':
                if (optopt == 'c')
                    fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                else if (isprint(optopt))
                    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                else
                    fprintf(stderr,
                            "Unknown option character `\\x%x'.\n",
                            optopt);
                return false;
            default:
                return false;
            }
        }
    }
    return true;
}

Argument::Argument(char key,std::string long_name,bool has_parameter):
    long_name_(long_name), key_(key), has_parameter_(has_parameter){}