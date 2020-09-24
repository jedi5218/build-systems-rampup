#ifndef ARGSPARSER_HPP
#define ARGSPARSER_HPP

class ArgsParser
{
public:
    //ArgsParser(std::initializer_list<Argument&> arguments);
    //Argument& operator[](char symbol) const;

};


class Argument
{
public:

};

class Flag: public Argument
{

};

class Parameter: public Argument
{

};
void say_hello();

#endif //ARGSPARSER_HPP