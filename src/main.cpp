
#include "command_line_interface.h"

#include <string>
#include <memory>
#include <vector>

////////////////////////////////////////////////

enum class CliArgType
{
    Flag,
    Sequence
};

enum class CliArgId
{
    Help,
    File
};

using CliArgTemplate = CommandLineArgumentTemplate< CliArgType, CliArgId>;
using CliArgDict = CommandLineArgumentDict< CliArgId>;
using CliArgParser = CommandLineArgumentParser< CliArgTemplate>;

int main( int argc, char* argv[])
{
    ////////////////////////////////////////////////

    std::vector< CliArgTemplate> argTemplates
    {
        CliArgTemplate
        {
            CliArgType::Flag, CliArgId::Help
        },
        CliArgTemplate
        {
            CliArgType::Sequence, CliArgId::File
        }
    };

    CliArgParser argParser{ argc, argv};
    CliArgDict argDict;

    if( ! argParser.load_argument_templates( std::views::all( argTemplates)))
        printf( "Loading command line argument definitions failed.\n");

    if( ! argParser.parse( argDict))
        printf( "Parsing command line failed.\n");

    if( argDict.get_key_value( CliArgId::Help))
        printf( "Help was invoked.\n");

    return 0;
}
