
#include "forward_string_index.h"
#include <memory>
#include <vector>

#define RC( T, X) reinterpret_cast< T>( X)
#define SC( T, X) static_cast< T>( X)
#define CC( T, X) const_cast< T>( X)

#define VANISH( X)
#define IDENTITY( X) X
#define APPEND_COMMA( X) X,

template< typename ArgTypeT, typename ArgIdT>
struct CommandLineArgumentTemplate
{
    ArgTypeT            Type;
    ArgIdT              Id;
    std::string         ShortName;
    std::string         LongName;
    std::string         HelpInfo;
};

enum class CliArgType
{
    Flag,
    Sequence
};

#define CLI_ARG_DEFS( WRAPPER, TYPE, ID, SNAME, LNAME, HELP)\
    WRAPPER( TYPE( Flag)            ID( Help)           SNAME( "h")     LNAME( "help")      HELP( "Displays this message."))\
    WRAPPER( TYPE( Sequence)        ID( File)           SNAME( "f")     LNAME( "files")     HELP( "Files to perform search on."))


enum class CliArgId
{
    CLI_ARG_DEFS( IDENTITY, VANISH, APPEND_COMMA, VANISH, VANISH, VANISH)
};

using CliArgTemplate = CommandLineArgumentTemplate< CliArgType, CliArgId>;

#define DECL_CLI_ARG_TEMPLATE( X) CliArgTemplate{ X},
#define PASS_TYPE( X) CliArgType::X
#define PASS_ID( X) , CliArgId::X
#define PASS_TEXT( X) , X

static std::vector< CliArgTemplate> const CommandLineArgumentTemplateList
{
    CLI_ARG_DEFS( DECL_CLI_ARG_TEMPLATE, PASS_TYPE, PASS_ID, PASS_TEXT, PASS_TEXT, PASS_TEXT)
};

#undef PASS_TEXT
#undef PASS_ID
#undef PASS_TYPE
#undef DECL_CLI_ARG_TEMPLATE

int main( int /*argc*/, char* /*argv*/[])
{
    ForwardStringIndexFactory< CliArgId> idxFactory;

    for( CliArgTemplate const & argTemplate : CommandLineArgumentTemplateList)
        if( ! idxFactory.add( argTemplate.ShortName.data(), CC( CliArgId *, &argTemplate.Id)))
            printf( "Cli argument short name: `%s` was not indexed due to conflicts.\n", argTemplate.ShortName.data());

    std::unique_ptr< ForwardStringIndex< CliArgId>> shortNameIndex{ idxFactory.emit()};

    ////////////////////////////////////////////////

    ForwardStringIndexFactory< char> testFactory;

    std::vector< char const *> testOpt{ "a", "aa", "ab", "baaa", "baab", "baa", "bb", "bb", "cccBcOcc", "cccBccccc", "cccDccc"};

    for( char const * opt : testOpt)
    {
        if( ! testFactory.add( opt, CC( char *, opt)))
            printf( "fail: %s\n", opt);
    }

    std::unique_ptr< ForwardStringIndex< char>> testIdx{ testFactory.emit()};

    printf( "%s\n", testIdx->find( "cccBcOcc"));
    printf( "%s\n", testIdx->find( "cccBccccc"));
    printf( "%s\n", testIdx->find( "cccDccc"));

    return 0;
}
