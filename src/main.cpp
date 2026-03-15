
#include "fwdstridx.h"
#include <memory>
#include <vector>

#define RC( T, X) reinterpret_cast< T>( X)
#define SC( T, X) static_cast< T>( X)

template< typename ArgId, typename ArgType>
struct CommandLineArgumentTemplate
{
    ArgId                 Id;
    ArgType               Type;
    const std::string     ShortName;
    const std::string     LongName;
    const std::string     HelpInfo;
};

enum class CliArgType
{
    Flag,
    Sequence
};

#define CLI_ARG_DEFS( WRAPPER, ID, TYPE, SNAME, LNAME, HELP)\
    WRAPPER( ID( Help)          TYPE( CliArgType::Flag)             SNAME( "h")     LNAME( "help")      HELP( "Displays this message.")) \
    WRAPPER( ID( File)          TYPE( CliArgType::Sequence)         SNAME( "f")     LNAME( "files")     HELP( "Files to perform search on."))


#define VANISH( X)
#define IDENTITY( X) X
#define APPEND_COMMA( X) X,

enum class CliArgId
{
    CLI_ARG_DEFS( IDENTITY, APPEND_COMMA, VANISH, VANISH, VANISH, VANISH)
};

using CliArgTemplate = CommandLineArgumentTemplate< CliArgId, CliArgType>;


#define DECL_CLI_ARG_TEMPLATE( X) CliArgTemplate{ X},
#define PASS_ENUM( X) CliArgId::X
#define PASS_TEXT( X) , X

static std::vector< CliArgTemplate> const CommandLineArgTemplates
{
    CLI_ARG_DEFS( DECL_CLI_ARG_TEMPLATE, PASS_ENUM, PASS_TEXT, PASS_TEXT, PASS_TEXT, PASS_TEXT)
};

int main( int /*argc*/, char* /*argv*/[])
{
    ForwardStringIndexFactory idxFactory;

    std::unique_ptr< ForwardStringIndex> fsi{ idxFactory.emit()};

    printf( "%ld\n", RC( long, fsi->find( "h", 1)));
    printf( "%ld\n", RC( long, fsi->find( "AB", 2)));
    printf( "%ld\n", RC( long, fsi->find( "AC", 2)));


    return 0;
}
