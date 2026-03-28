
#include "command_line_interface.hpp"

#include <cstdio>
#include <string>
#include <vector>
#include <iostream>

////////////////////////////////////////////////

enum class CliArgType
{
    Flag,
    Sequence
};

enum class CliArgId
{
    Help,
    File,
    Filee
};

using CliArgTemplate = CommandLineArgumentTemplate< CliArgType, CliArgId>;
using CliArgDict = CommandLineArgumentDict< CliArgId>;
using CliArgParser = CommandLineArgumentParser< CliArgTemplate>;

#define INIT_ARG_TEMPLATE( ArrayLike, Id, Type, ShortKey, LongKey) \
    do {\
        auto & _argtmplt = ArrayLike[ static_cast< long>( Id)];\
        _argtmplt.m_ArgType = (Type);\
        _argtmplt.m_ShortKey = (ShortKey);\
        _argtmplt.m_LongKey = (LongKey);\
    } while( 0)

int main( int argc, char const * argv[])
{
    ////////////////////////////////////////////////

    std::vector< CliArgTemplate> argTemplates{
        CliArgTemplate{ CliArgId::Help},
        CliArgTemplate{ CliArgId::File},
        CliArgTemplate{ CliArgId::Filee},
    };

    INIT_ARG_TEMPLATE( argTemplates, CliArgId::Help, CliArgType::Flag, "h", "help");
    INIT_ARG_TEMPLATE( argTemplates, CliArgId::File, CliArgType::Sequence, "f", "file");
    INIT_ARG_TEMPLATE( argTemplates, CliArgId::Filee, CliArgType::Sequence, "f", "fisle");

    CliArgParser argParser{ argc, argv, std::views::all( argTemplates)};
    CliArgDict argDict;

    if( ! argParser.parse( argDict))
        printf( "Parsing command line failed.\n");

    // ahh te dedykowane obsługi znaków w konsolach...
    // run -hf testa testb testc --testd teste --file \'\|testf\ \ \ \|\'

    if( argDict.get_argument_value( CliArgId::Help))
        printf( "Help was invoked.\n");

    if( CliArgDict::const_iterator fileValue = argDict.get_argument_value( CliArgId::File); fileValue != argDict.end())
        for( std::string const & fileText : *fileValue)
            std::cout<< fileText << ' ';

    std::cout << '\n';

    return 0;
}
