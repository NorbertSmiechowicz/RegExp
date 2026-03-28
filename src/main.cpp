
#include "command_line_interface.hpp"

#include <cstdio>
#include <string>
#include <iostream>
#include <array>

////////////////////////////////////////////////

enum class CliArgType
{
    Flag,
    Sequence
};

#define CLI_ARG_LIST_DEF\
    CLI_ARG_LIST_ITEM( CliArgType::Flag,         Help,       "h",    "help",     "",     "")\
    CLI_ARG_LIST_ITEM( CliArgType::Sequence,     File,       "f",    "file",     "",     "")

#define CLI_ARG_LIST_ITEM( TYPE, ID, ...) ID,

enum class CliArgId
{
    CLI_ARG_LIST_DEF
    CliArgIdCount
};

#undef CLI_ARG_LIST_ITEM

using CliArgTemplate = CommandLineArgumentTemplate< CliArgType, CliArgId>;
using CliArgParser = CommandLineArgumentParser< CliArgTemplate>;

#define CLI_ARG_LIST_ITEM( TYPE, ID, ...) CliArgTemplate{ TYPE, CliArgId::ID, __VA_ARGS__},

static constexpr std::array< CliArgTemplate, static_cast< std::size_t>( CliArgId::CliArgIdCount)> argTemplates{
    CLI_ARG_LIST_DEF
};

#undef CLI_ARG_LIST_ITEM
#undef CLI_ARG_LIST_DEF

////////////////////////////////////////////////

int main( int argc, char const * argv[])
{
    std::optional< CliArgParser::Dict> argDict = CliArgParser{ argc, argv, argTemplates}.parse();

    if( ! argDict.has_value())
        return 1;

    // ahh te dedykowane obsługi znaków w konsolach...
    // run -hf testa testb testc --testd teste --file \'\|testf\ \ \ \|\'

    if( argDict->get_argument_value( CliArgId::Help))
        printf( "Help was invoked.\n");

    if( CliArgParser::Dict::const_iterator fileValue = argDict->get_argument_value( CliArgId::File); fileValue != argDict->end())
        for( std::string const & fileText : *fileValue)
            std::cout<< fileText << ' ';

    std::cout << '\n';

    return 0;
}
