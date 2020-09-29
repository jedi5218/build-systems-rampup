#ifndef ARGSPARSER_HPP
#define ARGSPARSER_HPP
#include <initializer_list>
#include <functional>
#include <string>
#include <unordered_map>

class Argument;

using  ArgumentList = std::initializer_list<std::reference_wrapper<Argument>>;
using ArgumentDict = std::unordered_map<char,std::reference_wrapper<Argument>>;

class ArgsParser
{
public:
    ArgsParser(ArgumentList arguments, const char* help);
    bool parse(int argc, char* argv[]);
private:
    ArgumentDict arguments_;
    std::string short_options;
    const char* help_text;
};


class Argument
{
public:

    Argument(char key,std::string long_name,bool has_value);
    bool has_parameter() const {return has_parameter_;}
    bool is_set() const {return is_set_;}
    std::string value() const {return value_;}
    inline std::string long_name() const {return long_name_;}
    inline char key() const {return key_;}

protected:

    std::string long_name_;
    char key_;
    bool has_parameter_;
    bool is_set_ = false;
    std::string value_;
    friend class ArgsParser;
};

void say_hello();

#endif //ARGSPARSER_HPP